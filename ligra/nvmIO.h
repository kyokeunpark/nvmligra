#pragma once
#include "IO.h"
#include <libpmem.h>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/utils.hpp>

#define newP(__E,__n) (__E *) pmemmgr.allocate((__n) * sizeof(__E))

using namespace std;

template <class vertex>
class PMemManager {
  void *pmemaddr;
  size_t size;
  size_t allocated = 0;
  int is_pmem;

public:
  PMemManager(char *filename, size_t pmemsize)  {
    if ((pmemaddr = pmem_map_file(filename, pmemsize, PMEM_FILE_CREATE, 0666, &size, &is_pmem)) == NULL) {
      perror("pmem_map_file");
      abort();
    } else if (!is_pmem) {
      cerr << "Warning: " << filename << " is not a persistent memory." << endl;
    }
  }

  ~PMemManager() {
    pmem_unmap(pmemaddr, size);
  }

  // TODO
  void *allocate(size_t length) {
    void *tmp = ((char *) pmemaddr + allocated);
    allocated += length;
    return tmp;
  }
};

template <class vertex>
nvmgraph<vertex> readNvmgraphFromFile(char* fname, bool isSymmetric, bool mmap, char *pmemname, size_t pmemsize) {
  words W;
  PMemManager<vertex> pmemmgr = PMemManager<vertex>(pmemname, pmemsize);

  if (mmap) {
    _seq<char> S = mmapStringFromFile(fname);
    char *bytes = newA(char, S.n);
    // Cannot mutate the graph unless we copy.
    parallel_for(size_t i = 0; i < S.n; i++) {
      bytes[i] = S.A[i];
    }
    if (munmap(S.A, S.n) == -1) {
      perror("munmap");
      exit(-1);
    }
    S.A = bytes;
    W = stringToWords(S.A, S.n);
  } else {
    _seq<char> S = readStringFromFile(fname);
    W = stringToWords(S.A, S.n);
  }
#ifndef WEIGHTED
  if (W.Strings[0] != (string) "AdjacencyGraph") {
#else
  if (W.Strings[0] != (string) "WeightedAdjacencyGraph") {
#endif
    cout << "Bad input file" << endl;
    cout << W.Strings[0] << endl;
    abort();
  }

  long len = W.m - 1;
  long n = atol(W.Strings[1]);
  long m = atol(W.Strings[2]);
#ifndef WEIGHTED
  if (len != n + m + 2) {
#else
  if (len != n + 2*m + 2) {
#endif
    cout << "Bad input file" << endl;
    abort();
  }

  // Allocating space for edges
  // TODO: Use Pmem here
  uintT* offsets = newA(uintT,n);
  // uintT* offsets = newP(uintT, n);
#ifndef WEIGHTED
  // uintE* edges = newA(uintE, m);
  uintE* edges = newP(uintE, m);
#else
  // intE* edges = newA(intE,2*m);
  intE* edges = newP(intE, 2*m);
#endif

  // Retrieving offsets from file
  {parallel_for(long i = 0; i < n; i++)
      offsets[i] = atol(W.Strings[i + 3]);}
  {parallel_for(long i = 0; i < m; i++) {
#ifndef WEIGHTED
      edges[i] = atol(W.Strings[i+n+3]);
#else
      edges[2*i] = atol(W.Strings[i+n+3]);
      edges[2*i+1] = atol(W.Strings[i+n+m+3]);
#endif
    }}

  vertex* v = newA(vertex,n);

  {parallel_for (uintT i = 0; i < n; i++) {
      uintT o = offsets[i];
      uintT l = ((i == n - 1) ? m : offsets[i+1]) - offsets[i];
      v[i].setOutDegree(l);
#ifndef WEIGHTED
      v[i].setOutNeighbors(edges + o);
#else
      v[i].setOutNeighbors(edges + 2 * o);
#endif
    }}

  if (!isSymmetric) {
    uintT* tOffsets = newA(uintT,n);
    {parallel_for(long i = 0; i < n; i++)
        tOffsets[i] = INT_T_MAX;}
#ifndef WEIGHTED
    intPair* temp = newA(intPair,m);
#else
    intTriple* temp = newA(intTriple,m);
#endif
    {parallel_for(long i = 0; i < n; i++) {
        uintT o = offsets[i];
        for(uintT j = 0; j < v[i].getOutDegree(); j++) {
#ifndef WEIGHTED
          temp[o+j] = make_pair(v[i].getOutNeighbor(j), i);
#else
          temp[o+j] = make_pair(v[i].getOutNeighbor(j), make_pair(i, v[i].getOutWeight(j)));
#endif
        }
      }}
    free(offsets);

#ifndef WEIGHTED
#ifndef LOWMEM
    intSort::iSort(temp, m, n + 1, getFirst<uintE>());
#else
    quickSort(temp, m, pairFirstCmp<uintE>());
#endif
#else
#ifndef LOWMEM
    intSort::iSort(temp, m, n + 1, getFirst<intPair>());
#else
    quickSort(temp, m, pairFirstCmp<intPair>());
#endif
#endif

    tOffsets[temp[0].first] = 0;
#ifndef WEIGHTED
    // uintE* inEdges = newA(uintE,m);
    uintE* inEdges = newP(uintE, m);
    inEdges[0] = temp[0].second;
#else
    // intE* inEdges = newA(intE,2*m);
    intE* inEdges = newP(intE, 2*m);
    inEdges[0] = temp[0].second.first;
    inEdges[1] = temp[0].second.second;
#endif
    {parallel_for(long i=1;i<m;i++) {
#ifndef WEIGHTED
      inEdges[i] = temp[i].second;
#else
      inEdges[2*i] = temp[i].second.first;
      inEdges[2*i+1] = temp[i].second.second;
#endif
      if(temp[i].first != temp[i-1].first) {
	tOffsets[temp[i].first] = i;
      }
      }}

    free(temp);

    //fill in offsets of degree 0 vertices by taking closest non-zero
    //offset to the right
    sequence::scanIBack(tOffsets,tOffsets,n,minF<uintT>(),(uintT)m);

    {parallel_for(long i=0;i<n;i++){
      uintT o = tOffsets[i];
      uintT l = ((i == n-1) ? m : tOffsets[i+1])-tOffsets[i];
      v[i].setInDegree(l);
#ifndef WEIGHTED
      v[i].setInNeighbors(inEdges+o);
#else
      v[i].setInNeighbors(inEdges+2*o);
#endif
      }}

    free(tOffsets);
    Uncompressed_Mem<vertex>* mem = new Uncompressed_Mem<vertex>(v,n,m,edges,inEdges);
    return nvmgraph<vertex>(v,n,m,mem);
  }
  else {
    free(offsets);
    Uncompressed_Mem<vertex>* mem = new Uncompressed_Mem<vertex>(v,n,m,edges);
    return nvmgraph<vertex>(v,n,m,mem);
  }
}

template <class vertex>
nvmgraph<vertex> readNvmGraph(char* iFile, bool compressed, bool symmetric, bool binary, bool mmap, char* pmem, size_t pmemsize) {
  if (binary) {
    cerr << "NVM Graph for binary is not supported" << endl;
    abort();
  }
  return readNvmgraphFromFile<vertex>(iFile, symmetric, mmap, pmem, pmemsize);
}
