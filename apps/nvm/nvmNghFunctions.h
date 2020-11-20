   // Used by edgeMapSparse_no_filter. Sequentially decode the out-neighbors,
  // and compactly write all neighbors satisfying g().
  template <class V, class F, class G, class vertex>
  inline size_t decodeOutNghSparseSeq(V* v, long i, uintT o, F &f, G &g, nvmgraph<vertex> GA) {
    uintE d = v->getOutDegree();
    size_t k = 0;
    for (size_t j=0; j<d; j++) {
      uintE ngh = GA.E[o + j].to;
      if (f.cond(ngh)) {
#ifndef WEIGHTED
        auto m = f.updateAtomic(i, ngh);
#else
        auto m = f.updateAtomic(i, ngh, GA.E[o + j].weight);
#endif
        bool wrote = g(ngh, o+k, m);
        if (wrote) { k++; }
      }
    }
    return k;
  }
 
 
  // Used by edgeMapSparse. For each out-neighbor satisfying cond, call
  // updateAtomic.
  template <class V, class F, class G, class vertex>
  inline void decodeOutNghSparse(V* v, long i, uintT o, F &f, G &g, nvmgraph<vertex> GA) {
    uintE d = v->getOutDegree();
    granular_for(j, 0, d, (d > 1000), {
      uintE ngh = GA.E[o + j].to;
      if (f.cond(ngh)) {
#ifndef WEIGHTED
        auto m = f.updateAtomic(i, ngh);
#else
        auto m = f.updateAtomic(i, ngh, GA.E[o + j].weight);
#endif
        g(ngh, o+j, m);
      } else {
        g(ngh, o+j);
      }
    });
  }


// Used by edgeMapDenseForward. For each out-neighbor satisfying cond, call
// updateAtomic.
template <class V, class F, class G, class vertex>
inline void decodeOutNgh(V *v, long i, F &f, G &g, nvmgraph<vertex> GA) {
  uintE d = v->getOutDegree();
  uintT o = v->getOffset();
  granular_for(j, 0, d, (d > 1000), {
    uintT index = o + j;
    uintE ngh = GA.E[index].to;
    if (f.cond(ngh)) {
#ifndef WEIGHTED
      auto m = f.updateAtomic(i, ngh);
#else
      auto m = f.updateAtomic(i,ngh, GA.E[index].weight);
#endif
      g(ngh, m);
    }
  });
}

/*any way to be faster to find incoming edge and the neighbors?*/
template <class vertex, class F, class G, class VS>
inline void decodeInNghBreakEarly(long v_id, VS &vertexSubset, F &f,
                                  G &g, nvmgraph<vertex> GA,
                                  bool parallel = 0) {
//   uintE d = GA.m;
//   if (!parallel || d < 1000) {
//     for (size_t j = 0; j < d; j++) {
//       uintE ngh = GA.E[j].from;
//       if (GA.E[j].to == v_id && vertexSubset.isIn(ngh)) {
// #ifndef WEIGHTED
//         auto m = f.update(ngh, v_id);
// #else
//         auto m = f.update(ngh, v_id, GA.E[j].weight);
// #endif
//         g(v_id, m);
//       }
//       // if (!f.cond(v_id)) break;
//     }
//   } else {
//     parallel_for(size_t j = 0; j < d; j++) {
//       uintE ngh = GA.E[j].from;
//       if (GA.E[j].to == v_id && vertexSubset.isIn(ngh)) {
// #ifndef WEIGHTED
//         auto m = f.updateAtomic(ngh, v_id);
// #else
//         auto m = f.updateAtomic(ngh, v_id, GA.E[j].weight);
// #endif
//         g(v_id, m);
//       }
//     }
//   }
  granular_for(j, 0, GA.m, (GA.m > 1000), {
    uintE ngh = GA.E[j].from;
    if(GA.E[j].to == v_id && vertexSubset.isIn(ngh)){
#ifndef WEIGHTED
      auto m = f.updateAtomic(ngh, v_id);
#else
      auto m = f.updateAtomic(ngh, v_id, GA.E[j].weight);
#endif
      g(v_id, m);
    }
  });
}
