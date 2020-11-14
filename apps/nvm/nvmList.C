#define NVM 1
#include <iostream>
#include "ligra.h"
#include "nvmIO.h"

using namespace std;

template <class vertex>
void Compute(nvmgraph<vertex>& GA, commandLine P) {
  for (uintT i = 0; i < GA.n; i++) {
    vertex V = GA.V[i];
    uintT o = V.getOffset();
    uintT deg = V.getOutDegree();

    cout << "Vertex: " << i << ", deg: " << deg << endl;
    for (long j = o; j < o + deg; j++) {
#ifndef WEIGHTED
      cout << "(from: " << GA.E[j].from << ",to: " << GA.E[j].to << ")" << endl;
#else
      cout << "(from: " << GA.E[j].from << ",to: " << GA.E[j].to << ", weight: " << GA.E[j].weight << ")" << endl;
#endif
    }

  }
}
