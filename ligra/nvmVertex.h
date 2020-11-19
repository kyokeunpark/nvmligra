#pragma once
#include "ligra.h"
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

struct nvmSymmetricVertex {
  uintT degree, offset;

  void del() { }
  uintT getInDegree() const { return degree; }
  uintT getOutDegree() const { return degree; }
  uintT getOffset() const { return offset; }
  void setInDegree(uintT _d) { degree = _d; }
  void setOutDegree(uintT _d) { degree = _d; }
  void setOffset(uintT _o) { offset = _o; }
  void flipEdges() { /* TODO */ }
  template <class F, class G>
  inline void decodeOutNgh(long i, F &f, G& g) {
     decode_uncompressed::decodeOutNgh<nvmSymmetricVertex, F, G>(this, i, f, g);
  }
};

struct nvmAsymmetricVertex {
  uintT inDegree, outDegree, offset;

  void del() { }
  uintT getInDegree() const { return inDegree; }
  uintT getOutDegree() const { return outDegree; }
  uintT getOffset() const { return offset; }
  void setInDegree(uintT _d) { inDegree = _d; }
  void setOutDegree(uintT _d) { outDegree = _d; }
  void setOffset(uintT _o) { offset = _o; }
  void flipEdges() { /* TODO */ }
  template <class F, class G>
  inline void decodeOutNgh(long i, F &f, G &g) {
    decode_uncompressed::decodeOutNgh<nvmAsymmetricVertex, F, G>(this, i, f, g);
  }
};


  