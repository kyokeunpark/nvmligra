#pragma once
#include "IO.h"

using namespace std;

template <class vertex>
nvmgraph<vertex> readNvmgraphFromFile(char* fname, bool isSymmetric, bool mmap) {
  words W;
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
  if (W.Strings[0] != (string) "Adjacencygraph") {
#else
  if (W.Strings[0] != (string) "WeightedAdjacencygraph") {
#endif
    cout << "Bad input file" << endl;
    abort();
  }

  long len = W.m - 1;
  long n = atol(W.Strings[1]);
  long m = atol(W.Strings[2]);
}
