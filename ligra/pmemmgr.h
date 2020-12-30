#pragma once

#include <libpmem.h>

using namespace std;

struct PMemManager {
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
    if (allocated > size) {
      cerr << "Warning: Memory allocation exceeded pmem size" << endl;
    }
    return tmp;
  }

  void del() {
    // pmem_unmap(pmemaddr, size);
  }
};
