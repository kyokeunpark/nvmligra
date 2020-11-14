#ifndef GRAPH_H
#define GRAPH_H
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <libpmem.h>
#include "vertex.h"
#include "compressedVertex.h"
#include "nvmVertex.h"
#include "parallel.h"
using namespace std;

// **************************************************************
//    ADJACENCY ARRAY REPRESENTATION
// **************************************************************

// Class that handles implementation specific freeing of memory
// owned by the graph
struct Deletable {
public:
  virtual void del() = 0;
};

template <class vertex>
struct Uncompressed_Mem : public Deletable {
public:
  vertex* V;
  long n;
  long m;
  void* allocatedInplace, * inEdges;

  Uncompressed_Mem(vertex* VV, long nn, long mm, void* ai, void* _inEdges = NULL)
  : V(VV), n(nn), m(mm), allocatedInplace(ai), inEdges(_inEdges) { }

  void del() {
    if (allocatedInplace == NULL)
      for (long i=0; i < n; i++) V[i].del();
    else free(allocatedInplace);
    free(V);
    if(inEdges != NULL) free(inEdges);
  }
};

template <class vertex>
struct Uncompressed_MemNVM : public Deletable {
public:
  vertex *V;
  void *E;
  long n;
  long m;

  Uncompressed_MemNVM(vertex *VV, void *EE, long nn, long mm) : V(VV), E(EE), n(nn), m(mm) {
  }

  void del() {
    free(V);
    free(E);
  }
};

template <class vertex>
struct Uncompressed_Memhypergraph : public Deletable {
public:
  vertex* V;
  vertex* H;
  long nv;
  long mv;
  long nh;
  long mh;
  void* edgesV, *inEdgesV, *edgesH, *inEdgesH;

 Uncompressed_Memhypergraph(vertex* VV, vertex* HH, long nnv, long mmv, long nnh, long mmh, void* _edgesV, void* _edgesH, void* _inEdgesV = NULL, void* _inEdgesH = NULL)
   : V(VV), H(HH), nv(nnv), mv(mmv), nh(nnh), mh(mmh), edgesV(_edgesV), edgesH(_edgesH), inEdgesV(_inEdgesV), inEdgesH(_inEdgesH) { }

  void del() {
    free(edgesV);
    free(edgesH);
    free(V);
    free(H);
    if(inEdgesV != NULL) free(inEdgesV);
    if(inEdgesH != NULL) free(inEdgesH);
  }
};

template <class vertex>
struct Compressed_Mem : public Deletable {
public:
  vertex* V;
  char* s;

  Compressed_Mem(vertex* _V, char* _s) :
                 V(_V), s(_s) { }

  void del() {
    free(V);
    free(s);
  }
};

template <class vertex>
struct Compressed_Memhypergraph : public Deletable {
public:
  vertex* V;
  vertex* H;
  char* s;

 Compressed_Memhypergraph(vertex* _V, vertex* _H, char* _s) :
  V(_V), H(_H), s(_s) { }

  void del() {
    free(V);
    free(H);
    free(s);
  }
};

template <class vertex>
struct graph {
  vertex *V;
  long n;
  long m;
  bool transposed;
  uintE* flags;
  Deletable *D;

graph(vertex* _V, long _n, long _m, Deletable* _D) : V(_V), n(_n), m(_m),
  D(_D), flags(NULL), transposed(0) {}

graph(vertex* _V, long _n, long _m, Deletable* _D, uintE* _flags) : V(_V),
  n(_n), m(_m), D(_D), flags(_flags), transposed(0) {}

  void del() {
    if (flags != NULL) free(flags);
    D->del();
    free(D);
  }

  void transpose() {
    if ((sizeof(vertex) == sizeof(asymmetricVertex)) ||
        (sizeof(vertex) == sizeof(compressedAsymmetricVertex))) {
      parallel_for(long i=0;i<n;i++) {
        V[i].flipEdges();
      }
      transposed = !transposed;
    }
  }
};

struct PMemManager : public Deletable {
  void *pmemaddr;
  size_t size;
  size_t allocated = 0;
  int is_pmem;

public:
  PMemManager(char *filename, size_t pmemsize)  {
    if ((pmemaddr = pmem_map_file(filename, pmemsize, PMEM_FILE_CREATE, 0666, &size, &is_pmem)) == NULL) {
      perror("pmem_map_file");
      abort();
    } else if (!is_pmem) {
      cerr << "Warning: " << filename << " is not a persistent memory." << endl;
    }
  }

  // TODO: Incredibly simple allocator currently.
  //       Do we need something more complex?
  void *allocate(size_t length) {
    void *tmp = ((char *) pmemaddr + allocated);
    allocated += length;
    return tmp;
  }

  void del() {
    pmem_unmap(pmemaddr, size);
  }
};


template <class vertex>
struct nvmgraph {
  vertex *V;
  edge *E;
  long n;
  long m;
  bool transposed;
  uintE* flags;
  PMemManager* mem;

nvmgraph(vertex* _V, long _n, long _m, edge* _E, PMemManager* _mem) : V(_V), n(_n), m(_m),
    E(_E), mem(_mem), flags(NULL), transposed(0) {}


  void del() {
    if (flags != NULL) free(flags);
    mem->del();
  }

  void transpose() {
    if ((sizeof(vertex) == sizeof(asymmetricVertex)) ||
        (sizeof(vertex) == sizeof(compressedAsymmetricVertex))) {
      parallel_for(long i = 0; i < n; i++) {
        V[i].flipEdges();
      }
      transposed = !transposed;
    }
  }
};

template <class vertex>
struct hypergraph {
  vertex *V;
  vertex *H;
  long nv;
  long mv;
  long nh;
  long mh;
  bool transposed;
  uintE* flags;
  Deletable *D;

hypergraph(vertex* _V, vertex* _H, long _nv, long _mv, long _nh, long _mh, Deletable* _D) : V(_V), H(_H), nv(_nv), mv(_mv), nh(_nh), mh(_mh),
    D(_D), flags(NULL), transposed(0) {}

hypergraph(vertex* _V, vertex* _H, long _nv, long _mv, long _nh, long _mh,  Deletable* _D, uintE* _flags) : V(_V), H(_H),
    nv(_nv), mv(_mv), nh(_nh), mh(_mh), D(_D), flags(_flags), transposed(0) {}

  void del() {
    if (flags != NULL) free(flags);
    D->del();
    free(D);
  }
  void initFlags() {
    flags = newA(uintE,max(nv,nh));
    parallel_for(long i=0;i<max(nv,nh);i++) flags[i]=UINT_E_MAX;
  }
  
  void transpose() {
    if ((sizeof(vertex) == sizeof(asymmetricVertex)) ||
        (sizeof(vertex) == sizeof(compressedAsymmetricVertex))) {
      parallel_for(long i=0;i<nv;i++) {
        V[i].flipEdges();
      }
      parallel_for(long i=0;i<nh;i++) {
	H[i].flipEdges();
      }
      transposed = !transposed;
    }
  }
};
#endif
