#define NVM 1
#include "ligra.h"

//For flags array to store status of each vertex
enum {UNDECIDED,CONDITIONALLY_IN,OUT,IN};

//Uncomment the following line to enable checking for
//correctness. Currently the checker does not work with Ligra+.

//#define CHECK 1

#ifdef CHECK
template<class vertex>
bool checkMis(nvmgraph<vertex>& G, int* flags) {
  const intE n = G.n;
  bool correct = true;
  parallel_for (int i = 0; i < n; i++) {
    intE outDeg = G.V[i].getOutDegree();
    uintT offset = G.V[i].getOffset();
    intE numConflict = 0;
    intE numInNgh = 0;
    for (int j = 0; j < outDeg; j++) {
      intE ngh = GA.E[offset + j].to;
      if (flags[i] == IN && flags[ngh] == IN) {
        numConflict++;
      }
      else if (flags[ngh] == IN) {
        numInNgh++;
      }
    }
    if (numConflict > 0) {
      if(correct) CAS(&correct,true,false);
    } 
    if (flags[i] != IN && numInNgh == 0) {
      if(correct) CAS(&correct,true,false);
    }
  }
  return correct;
}
#endif

struct MIS_Update {
  int* flags;
  MIS_Update(int* _flags) : flags(_flags) {}
  inline bool update (uintE s, uintE d) {
    //if neighbor is in MIS, then we are out
    if(flags[d] == IN) {if(flags[s] != OUT) flags[s] = OUT;}
    //if neighbor has higher priority (lower ID) and is undecided, then so are we
    else if(d < s && flags[s] == CONDITIONALLY_IN && flags[d] < OUT)
      flags[s] = UNDECIDED;
    return 1;
  }
  inline bool updateAtomic (uintE s, uintE d) {
    if(flags[d] == IN) {if(flags[s] != OUT) flags[s] = OUT;}
    else if(d < s && flags[s] == CONDITIONALLY_IN && flags[d] < OUT) 
      flags[s] = UNDECIDED;
    return 1;
  }
  inline bool cond (uintE i) {return cond_true(i);}
};

struct MIS_Filter {
  int* flags;
  MIS_Filter(int* _flags) : flags(_flags) {}
  inline bool operator () (uintE i) {
    if(flags[i] == CONDITIONALLY_IN) { flags[i] = IN; return 0; } //vertex in MIS
    else if(flags[i] == OUT) return 0; //vertex not in MIS
    else { flags[i] = CONDITIONALLY_IN; return 1; } //vertex undecided, move to next round
  }
};

//Takes a symmetric graph as input; priority of a vertex is its ID.
template <class vertex>
void Compute(nvmgraph<vertex>& GA, commandLine P) {
  const intE n = GA.n;
  bool checkCorrectness = P.getOptionValue("-checkCorrectness");

  //flags array: UNDECIDED means "undecided", CONDITIONALLY_IN means
  //"conditionally in MIS", OUT means "not in MIS", IN means "in MIS"
  int* flags = newA(int,n);
  bool* frontier_data = newA(bool, n);
  {parallel_for(long i=0;i<n;i++) {
    flags[i] = CONDITIONALLY_IN;
    frontier_data[i] = 1;
  }}
  long round = 0;
  nvmVertexSubset Frontier(n, frontier_data);
  while (!Frontier.isEmpty()) {
    nvmEdgeMap(GA, Frontier, MIS_Update(flags), -1, no_output);
    nvmVertexSubset output = vertexFilter(Frontier, MIS_Filter(flags));
    Frontier.del();
    Frontier = output;
    round++;
  }
#ifdef CHECK
  if (checkCorrectness) {
    if(checkMis(GA,flags)) cout << "correct\n";
    else cout << "incorrect\n";
  }
#endif
  free(flags);
  Frontier.del();
}

