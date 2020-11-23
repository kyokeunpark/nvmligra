#pragma once

#include <cstdlib>
#include "ligra.h"
#include "nvmIO.h"

/*
 * BST representation of edges that are cached in main memory
 *
 * TODO: Currently a simple BST implementation. This can be improved with
 *       self-balancing search tree, although currently not necessary.
 */
template <class vertex>
struct cachedEdge {
  uintT v;
  edge *e;
  struct cachedEdge<vertex> *left, *right;

public:
  cachedEdge<vertex>(uintT _v, edge *_e, cachedEdge<vertex> *l, cachedEdge<vertex> *r)
      : v(_v), e(_e), left(l), right(r) {}
  cachedEdge<vertex>(uintT _v, edge *_e) : v(_v), e(_e) {
    left = right = nullptr;
  }

  void addCachedEdge(cachedEdge<vertex> *n) {
    cachedEdge<vertex> *node = this;
    uintT v = n->v;

    while (node != nullptr) {
      if (node->v > v && node->left) {
        node = node->left;
      } else if (node->v > v) {
        node->left = n;
        break;
      } else if (node->v < v && node->right) {
        node = node->right;
      } else {
        node->right = n;
        break;
      }
    }
  }

  cachedEdge<vertex> *searchEdge(uintT v) {
    cachedEdge<vertex> *node = this;

    while (node != nullptr) {
      if (node->v == v)
        return node;
      else if (node->v > v)
        node = node->left;
      else
        node = node->right;
    }

    return nullptr;
  }

};

template <class vertex>
class nvmStream {
  const size_t buffer_size;
  edge *edges;
  void *bufaddr;
  cachedEdge<vertex> *cached;

public:
  nvmStream(const size_t bufsize, edge *_e) : buffer_size(bufsize), edges(_e) {
    bufaddr = malloc(buffer_size);
  }

  void *getBuffer() { return bufaddr; }

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
