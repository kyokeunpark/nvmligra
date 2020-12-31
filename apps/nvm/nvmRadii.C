#define NVM 1
#include "ligra.h"
#include "nvmIO.h"

//atomically do bitwise-OR of *a with b and store in location a
template <class ET>
inline void writeOr(ET *a, ET b) {
  volatile ET newV, oldV;
  do {
    oldV = *a;
    newV = oldV | b;
  } while ((oldV != newV) && !CAS(a, oldV, newV));
}

struct Radii_F {
  intE round;
  intE* radii;
  long* Visited, *NextVisited;
  Radii_F(long* _Visited, long* _NextVisited, intE* _radii, intE _round) :
    Visited(_Visited), NextVisited(_NextVisited), radii(_radii), round(_round)
  {}

  inline bool update (uintE s, uintE d){ //Update function does a bitwise-or
    long toWrite = Visited[d] | Visited[s];
    if(Visited[d] != toWrite){
      NextVisited[d] |= toWrite;
      if(radii[d] != round) { radii[d] = round; return 1; }
    }
    return 0;
  }

  inline bool updateAtomic (uintE s, uintE d){ //atomic Update
    long toWrite = Visited[d] | Visited[s];
    if(Visited[d] != toWrite){
      writeOr(&NextVisited[d],toWrite);
      intE oldRadii = radii[d];
      if(radii[d] != round) return CAS(&radii[d],oldRadii,round);
    }
    return 0;
  }

  inline bool cond (uintE d) { return cond_true(d); }
};

//function passed to vertex map to sync NextVisited and Visited
struct Radii_Vertex_F {
  long* Visited, *NextVisited;
  Radii_Vertex_F(long* _Visited, long* _NextVisited) :
    Visited(_Visited), NextVisited(_NextVisited) {}
  inline bool operator() (uintE i) {
    Visited[i] = NextVisited[i];
    return 1;
  }
};

template <class vertex>
void Compute(nvmgraph<vertex>& GA, commandLine P) {
  long n = GA.n;
  intE* radii = newA(intE,n);
  long* Visited = newA(long,n), *NextVisited = newA(long,n);
  {parallel_for(long i=0;i<n;i++) {
    radii[i] = -1;
    Visited[i] = NextVisited[i] = 0;
    }}
  long sampleSize = min(n,(long)64);
  uintE* starts = newA(uintE,sampleSize);

  {parallel_for(ulong i=0;i<sampleSize;i++) { //initial set of vertices
      uintE v = hashInt(i) % n;
    radii[v] = 0;
    starts[i] = v;
    NextVisited[v] = (long) 1<<i;
    }}

  nvmVertexSubset Frontier(n,sampleSize,starts); //initial frontier of size 64

  intE round = 0;
  while(!Frontier.isEmpty()){
    round++;
    nvmVertexMap(Frontier, Radii_Vertex_F(Visited,NextVisited));
    nvmVertexSubset output = nvmEdgeMap(GA,Frontier,Radii_F(Visited,NextVisited,radii,round));
    Frontier.del();
    Frontier = output;
  }
  free(Visited); free(NextVisited); Frontier.del(); free(radii);
}
