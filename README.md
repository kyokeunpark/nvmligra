NVM Optimized Graph
-------

This is a fork of Ligra which can also utilize Intel's Persistent Memory.
Examples of NVM-based Ligra application can be found in `/apps/nvm` directory.
In order for these to compile, you would need the following as well as
the base dependency:

- [PMDK](https://github.com/pmem/pmdk)
- [C++ binding for libpmemobj](https://github.com/pmem/libpmemobj-cpp)

You can find README for original Ligra [here](./README_old.md).
