#pragma once
#include "nvmVertexSubset.h"

struct edge {
#ifndef WEIGHTED
  uintE from, to;
  edge(uintE f, uintE t) : from(f), to(t) {}
#else
  intE from, to, weight;
  edge(intE f, intE t, intE w) : from(f), to(t), weight(w) {}
#endif
};

namespace decode_nvm {
    template <class V, class F, class G>
    inline size_t decodeOutNghSparseSeq(V* v, long i, uintT o, F &f, G &g) {
      uintE d = v->getOutDegree();
      size_t k = 0;
      for (size_t j=0; j<d; j++) {
        edge e = v->getOutNeighbor(j);
        uintE ngh = e.to;
        if (f.cond(ngh)) {
  #ifndef WEIGHTED
          auto m = f.updateAtomic(i, ngh);
  #else
          auto m = f.updateAtomic(i, ngh, e.weight);
  #endif
          bool wrote = g(ngh, o+k, m);
          if (wrote) { k++; }
        }
      }
      return k;
    }


    // Used by edgeMapSparse. For each out-neighbor satisfying cond, call
    // updateAtomic.
    template <class V, class F, class G>
    inline void decodeOutNghSparse(V* v, long i, uintT o, F &f, G &g) {
      uintE d = v->getOutDegree();
      granular_for(j, 0, d, (d > 1000), {
        edge e = v->getOutNeighbor(j);
        uintE ngh = e.to;
        if (f.cond(ngh)) {
  #ifndef WEIGHTED
          auto m = f.updateAtomic(i, ngh);
  #else
          auto m = f.updateAtomic(i, ngh, e.weight);
  #endif
          g(ngh, o+j, m);
        } else {
          g(ngh, o+j);
        }
      });
    }


  // Used by edgeMapDenseForward. For each out-neighbor satisfying cond, call
  // updateAtomic.
  template <class V, class F, class G>
  inline void decodeOutNgh(V *v, long i, F &f, G &g) {
    uintE d = v->getOutDegree();
    uintT o = v->getOffset();
    granular_for(j, 0, d, (d > 1000), {
      uintT index = o + j;
      edge e = v->getOutNeighbor(index);
      uintE ngh = e.to;
      if (f.cond(ngh)) {
  #ifndef WEIGHTED
        auto m = f.updateAtomic(i, ngh);
  #else
        auto m = f.updateAtomic(i,ngh, e.weight);
  #endif
        g(ngh, m);
      }
    });
  }

  /*any way to be faster to find incoming edge and the neighbors?*/
  template <class vertex, class F, class G, class VS>
  inline void decodeInNghBreakEarly(vertex *v, uintE v_id, VS &vertexSubset, F &f,
                                    G &g, bool parallel = 0) {
    uintT d = v->getInDegree();
    if (!parallel || d < 1000) {
      for (size_t j = 0; j < d; j++) {
        edge e = v->getInNeighbor(j);
        uintE ngh = e.to;
        if (vertexSubset.isIn(ngh)) {
  #ifndef WEIGHTED
          auto m = f.update(ngh, v_id);
  #else
          auto m = f.update(ngh, v_id, e.weight);
  #endif
          g(v_id, m);
        }
        if (!f.cond(v_id)) break;
      }
    } else {
      parallel_for(size_t j = 0; j < d; j++) {
        edge e = v->getInNeighbor(j);
        uintE ngh = v->getInNeighbor(j).to;
        if (vertexSubset.isIn(ngh)) {
  #ifndef WEIGHTED
          auto m = f.updateAtomic(ngh, v_id);
  #else
          auto m = f.updateAtomic(ngh, v_id, e.weight);
  #endif
          g(v_id, m);
        }
      }
    }
  }
};

struct nvmSymmetricVertex {
  edge *neighbors;
  uintT degree, offset;

  void del() { }
  edge *getInNeighbors () { return neighbors; }
  edge *getOutNeighbors () { return neighbors; }
  edge getInNeighbor(uintT j) { return neighbors[j]; }
  edge getOutNeighbor(uintT j) { return neighbors[j]; }
  uintT getInDegree() const { return degree; }
  uintT getOutDegree() const { return degree; }
  uintT getOffset() const { return offset; }
  void setInNeighbors(edge *_i) { neighbors = _i; }
  void setOutNeighbors(edge *_i) { neighbors = _i; }
  void setInNeighbor(uintT j, edge ngh) { neighbors[j] = ngh; }
  void setOutNeighbor(uintT j, edge ngh) { neighbors[j] = ngh; }
  void setInDegree(uintT _d) { degree = _d; }
  void setOutDegree(uintT _d) { degree = _d; }
  void setOffset(uintT _o) { offset = _o; }
  void flipEdges() { /* TODO */ }

  template <class VS, class F, class G>
  inline void decodeInNghBreakEarly(long v_id, VS& vertexSubset, F& f, G& g, bool parallel = 0) {
    decode_nvm::decodeInNghBreakEarly<nvmSymmetricVertex, F, G, VS>(this, v_id, vertexSubset, f, g, parallel);
  }

  template <class F, class G>
  inline void decodeOutNgh(long i, F &f, G& g) {
     decode_nvm::decodeOutNgh<nvmSymmetricVertex, F, G>(this, i, f, g);
  }
};

struct nvmAsymmetricVertex {
  edge *inNeighbors, *outNeighbors;
  uintT inDegree, outDegree, offset;

  void del() { }
  edge *getInNeighbors () { return inNeighbors; }
  edge *getOutNeighbors () { return outNeighbors; }
  edge getInNeighbor(uintT j) { return inNeighbors[j]; }
  edge getOutNeighbor(uintT j) { return outNeighbors[j]; }
  uintT getInDegree() const { return inDegree; }
  uintT getOutDegree() const { return outDegree; }
  uintT getOffset() const { return offset; }
  void setInNeighbors(edge *_i) { inNeighbors = _i; }
  void setOutNeighbors(edge *_i) { outNeighbors = _i; }
  void setInNeighbor(uintT j, edge ngh) { inNeighbors[j] = ngh; }
  void setOutNeighbor(uintT j, edge ngh) { outNeighbors[j] = ngh; }
  void setInDegree(uintT _d) { inDegree = _d; }
  void setOutDegree(uintT _d) { outDegree = _d; }
  void setOffset(uintT _o) { offset = _o; }
  void flipEdges() { /* TODO */ }

  template <class VS, class F, class G>
  inline void decodeInNghBreakEarly(long v_id, VS& vertexSubset, F& f, G& g, bool parallel = 0) {
    decode_nvm::decodeInNghBreakEarly<nvmAsymmetricVertex, F, G, VS>(this, v_id, vertexSubset, f, g, parallel);
  }

  template <class F, class G>
  inline void decodeOutNgh(long i, F &f, G &g) {
    decode_nvm::decodeOutNgh<nvmAsymmetricVertex, F, G>(this, i, f, g);
  }
};
