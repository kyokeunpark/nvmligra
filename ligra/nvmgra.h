/*
 * TODO: This will be used for various implementations (like BFS) later on
 */

#pragma once

#include "nvmIO.h"
#include "ligra.h"
#include "vertexSubset.h"

template <class data, class vertex, class VS, class F>
  vertexSubsetData<data> edgeMapData(nvmgraph<vertex>& GA, bool fromV, VS &vs, F f,
                                     intT threshold = -1, const flags& fl=0) {
  return vertexSubsetData<data>();
}
