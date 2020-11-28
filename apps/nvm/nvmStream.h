#pragma once

#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include "ligra.h"
#include "nvmIO.h"

/* Linked list to help keeping track of cached vertices */
struct llist {
  uintT vid;
  struct llist *next, *prev;
};

/* Cached vertex list manager */
class cachedList {
  llist *head = nullptr, *tail = nullptr;

public:
  ~cachedList() {
    llist *tmp = head;
    while (tmp) {
      llist *ttmp = tmp->next;
      free(tmp);
      tmp = ttmp;
    }
  }

  uintT pop_front() {
    uintT vid = head->vid;
    struct llist *tmp = head;
    head = head->next;
    free(tmp);
    return vid;
  }

  llist *push_back(uintT vid) {
    llist *newV = (llist *) malloc(sizeof(llist));
    newV->prev = tail;
    newV->next = nullptr;
    newV->vid = vid;
    if (!head) {
      head = tail = newV;
    } else {
      tail->next = newV;
      tail = newV;
    }

    return newV;
  }

  void renew(llist *v) {
    if (head == tail) return;

    if (v->prev)
      v->prev->next = v->next;
    if (v->next)
      v->next->prev = v->prev;

    v->prev = tail;
    v->next = nullptr;
    tail->next = v;
    tail = v;
  }

  void erase(llist *v) {
    if (v->prev)
      v->prev->next = v->next;
    if (v->next)
      v->next->prev = v->prev;
    free(v);
  }
};

struct eData {
  edge *e;
  size_t size;
  llist *index;
};

template <class vertex>
static inline size_t edgeSize(vertex *v) {
  return (sizeof(edge) * v->getOutDegree());
}

using eData = struct eData;
/* Key = vertex id, Value = pointer to edge in DRAM */
using emap = std::unordered_map<intT, eData>;

template <class vertex>
class nvmStream {
  const size_t buffer_size;
  const double threshold = 0.9; /* Threshold before eviction */
  size_t buf_used;
  emap cached; /* Hash map of cached edges */
  cachedList cached_list; /* List of cached vertices (for eviction) */
  edge *edges; /* Edge structs in pmem */
  long m;

  bool needEviction() {
    return buf_used > threshold * buffer_size;
  }

  void evictVertex(uintT vid) {
    auto search = cached.find(vid);
    if (search != cached.end()) {
      free(search->second.e);
      buf_used -= search->second.size;
      cached.erase(search);
    } else {
      std::cerr << "nvmStream: received request to evict vertex that is not cached (vid = " << vid << ")" << std::endl;
      abort(); // NOTE: It is possible that abort may not be necessary here.
    }
  }

  void evict() {
    while (needEviction()) {
      uintT v = cached_list.pop_front();
      evictVertex(v);
    }
  }

public:
  nvmStream(const size_t bufsize, nvmgraph<vertex> &GA) : buffer_size(bufsize), edges(GA.E), m(GA.m) {
    cached = emap();
    cached_list = cachedList();
    buf_used = 0;
  }

  ~nvmStream() {}

  void fetchVertex(uintT vid, vertex *v) {
    auto search = cached.find(vid);
    if (search == cached.end()) {
      size_t eSize = edgeSize(v);
      edge *e = malloc(eSize);
      eData dat = { .e = e, .size = eSize };

      dat.index = cached_list.push_back(vid);
      memcpy(e, edges + v->getOffset(), eSize);
      cached.emplace(vid, dat);
      buf_used += eSize;

      /* Garbage collection if needed */
      if (needEviction())
        evict();
    } else {
      eData dat = search.second;
      cached_list.renew(v);
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
