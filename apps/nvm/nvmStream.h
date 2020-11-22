#pragma once

#include <cstdlib>
#include "nvmIO.h"

class nvmStream {
  const size_t buffer_size;
  void *pmemaddr;
  void *dramaddr;

public:
  nvmStream(const size_t bufsize, void *_pmemaddr) : buffer_size(bufsize), pmemaddr(_pmemaddr) {
    dramaddr = malloc(buffer_size);
  }
};
