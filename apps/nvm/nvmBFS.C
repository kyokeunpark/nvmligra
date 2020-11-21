#define NVM 1
#include "ligra.h"
#include "nvmIO.h"
#include "nvmEdgeMapFunctions.h"
#include "nvmNghFunctions.h"
struct BFS_F {
  uintE* Parents;
  BFS_F(uintE* _Parents) : Parents(_Parents) {}
  inline bool update (uintE s, uintE d) { //Update
    if(Parents[d] == UINT_E_MAX) { Parents[d] = s; return 1; }
    else return 0;
  }
  inline bool updateAtomic (uintE s, uintE d){ //atomic version of Update
    return (CAS(&Parents[d],UINT_E_MAX,s));
  }
  //cond function checks if vertex has been visited yet
  inline bool cond (uintE d) { return (Parents[d] == UINT_E_MAX); }
};

// TODO: Fix this to be actually work with NVM graph
template <class vertex>
void Compute(nvmgraph<vertex>& GA, commandLine P) {
  long start = P.getOptionLongValue("-r",0);
  long n = GA.n;
  long counter = 0;
  //creates Parents array, initialized to all -1, except for start
  uintE* Parents = newA(uintE,n);
  parallel_for(long i=0;i<n;i++) Parents[i] = UINT_E_MAX;
  Parents[start] = start;
  nvmVertexSubset Frontier(n, start); //creates initial frontier
  std::cout << *Frontier.s << std::endl;
  while(!Frontier.isEmpty()){ //loop until frontier is empty
    nvmVertexSubset output = nvmEdgeMap(GA, Frontier, BFS_F(Parents));
    Frontier.del();
    Frontier = output; //set new frontier
    std::cout << Frontier.s << " " << Frontier.m << " " << Frontier.dense() << std::endl;
    counter++;
  }
  std::cout << "Counter: " << counter << std::endl;
  Frontier.del();
  free(Parents);
}