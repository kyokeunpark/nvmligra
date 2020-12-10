#pragma once
#include "IO.h"
#include "nvmVertex.h"
#include "parallel.h"
#include <libpmem.h>

#define newP(__E,__n) ((__E *) pmemmgr->allocate((__n) * sizeof(__E)))

using namespace std;

// parallel code for converting a string to words
words stringToWords(char *Str, long n, PMemManager* pmemmgr) {
  {parallel_for (long i=0; i < n; i++)
      if (isSpace(Str[i])) Str[i] = 0; }

  // mark start of words
  bool *FL = newA(bool,n);
  FL[0] = Str[0];
  {parallel_for (long i=1; i < n; i++) FL[i] = Str[i] && !Str[i-1];}

  // offset for each start of word
  _seq<long> Off = sequence::packIndex<long>(FL, n);
  long m = Off.n;
  long *offsets = Off.A;

  // pointer to each start of word
  char **SA = newP(char*, m);
  {parallel_for (long j=0; j < m; j++) SA[j] = Str+offsets[j];}

  free(offsets); free(FL);

  return words(Str,n,SA,m);
}

template <class vertex>
nvmgraph<vertex> readNvmgraphFromFile(char* fname, bool isSymmetric, bool mmap, char *pmemname, size_t pmemsize) {
  words W;
  bool largeVertex = false;
  PMemManager* pmemmgr = new PMemManager(pmemname, pmemsize);

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
    W = stringToWords(S.A, S.n, pmemmgr);
  } else {
    _seq<char> S = readStringFromFile(fname);
    W = stringToWords(S.A, S.n, pmemmgr);
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

  unsigned long len = W.m - 1;
  unsigned long n = atol(W.Strings[1]);
  unsigned long m = atol(W.Strings[2]);
#ifndef WEIGHTED
  if (len != n + m + 2) {
#else
  if (len != n + 2*m + 2) {
#endif
    cout << "Bad input file" << endl;
    abort();
  }

  // Allocating space for edges
  uintT* offsets = newA(uintT,n);

  // Retrieving offsets from file
  {parallel_for(unsigned long i = 0; i < n; i++)
      offsets[i] = atol(W.Strings[i + 3]);}

  vertex* v = newA(vertex, n);
  // If it struggles to allocate to memory, place it in pmem instead
  if (!v) {
    v = newP(vertex, n);
    largeVertex = true;
  }
  edge* e = newP(edge, m);

  {parallel_for (long i = 0; i < m; i++) {
#ifndef WEIGHTED
    e[i] = edge(i, atol(W.Strings[i + n + 3]));
#else
    e[i] = edge(i, atol(W.Strings[i + n + 3]), atol(W.Strings[i + n + m + 3]));
#endif
  }}

  {parallel_for (uintT i = 0; i < n; i++) {
    uintT o = offsets[i];
    v[i].setOffset(o);
    uintT l = ((i == n - 1) ? m : offsets[i + 1]) - offsets[i];
    v[i].setOutDegree(l);
    v[i].setOutNeighbors(e + o);
    }}

  if(!isSymmetric) {
    uintT* tOffsets = newA(uintT, n);
    {parallel_for(long i = 0; i < n; i++) tOffsets[i] = INT_T_MAX;}

#ifndef WEIGHTED
    intPair* temp = newP(intPair, m);
#else
    intTriple* temp = newP(intTriple, m);
#endif
    {parallel_for(unsigned long i = 0; i < n; i++) {
        uintT o = offsets[i];
        for(uintT j = 0; j < v[i].getOutDegree(); j++) {
#ifndef WEIGHTED
          temp[o + j] = make_pair(v[i].getOutNeighbor(j).to, i);
#else
          temp[o + j] = make_pair(v[i].getOutNeighbor(j).to, make_pair(i, v[i].getOutWeight(j)));
#endif
        }
      }}
    free(offsets);

#ifndef WEIGHTED
#ifndef LOWMEM
    intSort::iSort(temp,m,n+1,getFirst<uintE>());
#else
    quickSort(temp,m,pairFirstCmp<uintE>());
#endif
#else
#ifndef LOWMEM
    intSort::iSort(temp,m,n+1,getFirst<intPair>());
#else
    quickSort(temp,m,pairFirstCmp<intPair>());
#endif
#endif

    tOffsets[temp[0].first] = 0;
    // TODO: Persistent memory here
    edge* inE = newP(edge, m);
#ifndef WEIGHTED
    inE[0] = edge(temp[0].first, temp[0].second);
#else
    inE[0] = edge(temp[0].first, temp[0].second.first, temp[0].second.second);
#endif
    {parallel_for(long i = 1; i < m; i++) {
#ifndef WEIGHTED
        inE[i] = edge(temp[i].first, temp[i].second);
#else
        inE[i] = edge(temp[i].first, temp[i].second.first, temp[i].second.second);
#endif
        if (temp[i].first != temp[i - 1].first) {
          tOffsets[temp[i].first] = i;
        }
      }}

#ifndef WEIGHTED
    pmem_unmap(temp, sizeof(intPair) * m);
#else
    pmem_unmap(temp, sizeof(intTriple) * m);
#endif

    if (largeVertex)
      sequence::scanIBack(tOffsets, tOffsets, n, minF<uintT>(), (uintT)m, pmemmgr);
    else
      sequence::scanIBack(tOffsets, tOffsets, n, minF<uintT>(), (uintT)m);

    {parallel_for(long i = 0; i < n; i++) {
        uintT o = tOffsets[i];
        uintT l = ((i == n - 1) ? m : tOffsets[i + 1]) - tOffsets[i];
        v[i].setInDegree(l);
        v[i].setInNeighbors(inE + o);
      }}

    free(tOffsets);
    return nvmgraph<vertex>(v, n, m, e, pmemmgr);
  }

  free(offsets);
  return nvmgraph<vertex>(v, n, m, e, pmemmgr);
}

template <class vertex>
nvmgraph<vertex> readNvmGraph(char* iFile, bool compressed, bool symmetric, bool binary, bool mmap, char* pmem, size_t pmemsize) {
  if (binary) {
    cerr << "NVM Graph for binary is not supported" << endl;
    abort();
  }
  return readNvmgraphFromFile<vertex>(iFile, symmetric, mmap, pmem, pmemsize);
}
