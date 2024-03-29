ifdef LONG
INTT = -DLONG
endif

ifdef EDGELONG
INTE = -DEDGELONG
endif

ifdef PD
PD = -DPD
endif

ifdef BYTE
CODE = -DBYTE
else ifdef NIBBLE
CODE = -DNIBBLE
else
CODE = -DBYTERLE
endif

ifdef LOWMEM
MEM = -DLOWMEM
endif

PMEMCXX = $(shell pkg-config --cflags libpmem)
PMEMLIB = $(shell pkg-config --libs libpmem)

#compilers
ifdef CILK
PCC = g++
PCFLAGS = -std=c++17 -fcilkplus -lcilkrts -O3 -DCILK $(PMEMCXX) $(INTT) $(INTE) $(CODE) $(PD) $(MEM)
PLFLAGS = -fcilkplus -lcilkrts $(PMEMLIB)

else ifdef MKLROOT
PCC = icpc
PCFLAGS = -std=c++17 -O3 -DCILKP $(PMEMCXX) $(INTT) $(INTE) $(CODE) $(PD) $(MEM)
PLFLAGS = $(PMEMLIB)

else ifdef OPENMP
PCC = g++
PCFLAGS = -std=c++17 -fopenmp -march=native -O3 -DOPENMP $(PMEMCXX) $(INTT) $(INTE) $(CODE) $(PD) $(MEM)
PLFLAGS = $(PMEMLIB)

else
PCC = g++
PCFLAGS = -std=c++17 -O3 $(PMEMCXX) $(INTT) $(INTE) $(CODE) $(PD) $(MEM)
PLFLAGS = $(PMEMLIB)
endif

COMMON= ligra.h graph.h compressedVertex.h vertex.h utils.h IO.h parallel.h gettime.h index_map.h maybe.h sequence.h edgeMap_utils.h binary_search.h quickSort.h blockRadixSort.h transpose.h parseCommandLine.h byte.h byteRLE.h nibble.h byte-pd.h byteRLE-pd.h nibble-pd.h vertexSubset.h encoder.C decoder.C nvmIO.h nvmVertex.h pmemmgr.h nvmEdgeMap.h nvmVertexMap.h nvmVertexSubset.h

ALL= nvmBFS nvmList nvmPageRank nvmMIS nvmRadii

all: $(ALL)

nvmBFS: nvmBFS.C $(COMMON)
	$(PCC) $(PCFLAGS) -o $@ $< $(PLFLAGS)

nvmList: nvmList.C $(COMMON)
	$(PCC) $(PCFLAGS) -o $@ $< $(PLFLAGS)

nvmPageRank: nvmPageRank.C $(COMMON)
	$(PCC) $(PCFLAGS) -o $@ $< $(PLFLAGS)

nvmMIS: nvmMIS.C $(COMMON)
	$(PCC) $(PCFLAGS) -o $@ $< $(PLFLAGS)

nvmRadii: nvmRadii.C $(COMMON)
	$(PCC) $(PCFLAGS) -o $@ $< $(PLFLAGS)

% : %.C $(COMMON)
	$(PCC) $(PCFLAGS) -o $@ $<

$(COMMON):
	ln -s ../../ligra/$@ .

.PHONY : clean

clean :
	rm -f *.o $(ALL)

cleansrc :
	rm -f *.o $(ALL)
	rm $(COMMON)
