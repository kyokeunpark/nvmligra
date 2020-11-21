#pragma once
#include "IO.h"
#include "nvmVertex.h"
#include <libpmem.h>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/utils.hpp>

#define newP(__E,__n) ((__E *) pmemmgr->allocate((__n) * sizeof(__E)))

using namespace std;

template <class vertex>
nvmgraph<vertex> readNvmgraphFromFile(char* fname, bool isSymmetric, bool mmap, char *pmemname, size_t pmemsize) {
  words W;
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
  uintT* offsets = newA(uintT,n);

  // Retrieving offsets from file
  {parallel_for(long i = 0; i < n; i++)
      offsets[i] = atol(W.Strings[i + 3]);}

  vertex* v = newA(vertex, n);
  edge* e = newP(edge, m);

  {parallel_for (long i = 0; i < n; i++) {
    v[i].setOffset(offsets[i]);
    uintT l = ((i == n - 1) ? m : offsets[i + 1]) - offsets[i];
    v[i].setOutDegree(l);
    {parallel_for (long j = 0; j < l; j++) {
        uintT o = v[i].getOffset();
#ifndef WEIGHTED
        e[o + j] = edge(i, atol(W.Strings[j + o + n + 3]));
#else
        e[o + j] = edge(i, atol(W.Strings[j + o + n + 3]), atol(W.Strings[j + n + o + m + 3]));
#endif
    }}
  }}

  cout << e << endl;

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
