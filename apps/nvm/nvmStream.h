#pragma once

#include <cstdlib>
#include <cstring>
#include <map>
#include "ligra.h"
#include "nvmIO.h"

template <class vertex>
class nvmStream {
  /* Key = vertex id, Value = pointer to edge in DRAM */
  using map = std::map<intT, edge *>;

  const size_t buffer_size;
  map cached;
  char *cachedBitmap; /* Bitmap to show if vertex is cached */
  edge *edges; /* Edge structs in pmem */
  long m;
  void *bufaddr;

public:
  nvmStream(const size_t bufsize, nvmgraph<vertex> &GA) : buffer_size(bufsize), edges(GA.E), m(GA.m) {
    cachedBitmap = calloc(GA.n >> 3);
    bufaddr = malloc(buffer_size);
    /* If we can fit entire edges data in given buffer size, do so. */
    if (m * sizeof(edge) < buffer_size) {
      memcpy(bufaddr, edges, m * sizeof(edge));
      memset(cachedBitmap, ~0, GA.n >> 3);
    }
  }

  ~nvmStream() {
    free(bufaddr);
    free(cachedBitmap);
  }

  void fetchVertex(uintT vid, vertex *v) {
    auto search = cached.find(vid);
    if (search == cached.end()) {
      // TODO: Add edges to the buffer and make it point to that instead
      cached.emplace(vid, edges[v->getOffset()]);
    }
  }

  void evictVertex(uintT vid) {
    auto search = cached.find(vid);
    if (search != cached.end()) {
      // TODO: Free the cached edge (mark it as available)
      cached.erase(search);
    } else {
      std::cerr << "nvmStream: received request to evict vertex that is not cached (vid = " << vid << ")" << std::endl;
      abort(); // NOTE: It is possible that abort may not be necessary here.
    }
  }

  /*
   * Request to cache edges related to given vertex
   */
  void sendRequest(vertex *v) {
    uintT outDegree = v->getOutDegree();
#ifndef WEIGHTED
#else
#endif
  }
};
