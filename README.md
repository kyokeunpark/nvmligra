NVM Optimized Graph
-------

This is a fork of [Ligra](https://github.com/jshun/ligra) which can also utilize
Intel's Persistent Memory. This project was created as a course project for
CSC2224 University of Toronto. Examples of NVM-based Ligra application can
be found in `/apps/nvm` directory. In order for these to compile,
Intel's [PMDK](https://github.com/pmem/pmdk) must be installed, as well as
Ligra's base dependencies.

You can find README for original Ligra [here](./README_old.md) and you can
find the final report for CSC2224 [here](./csc2224_report.pdf).

Running code with NVM
-------

Applications with NVM-supported Ligra must specify path to the pmem file and
its size to be used as additional commandline argument. In order for
an application to use NVM-based Ligra implementation, `NVM` macro must be
defined.

``` sh
$ ./nvmBFS -p /path/to/pmem -S 123456 ../../inputs/test
```

Similar to other Ligra implementation, traversal applications can also pass
the "-r" flags followed by an integer to indicate the source vertex.

Note that all applications provided are only tested on Ubuntu 18.04 only.
