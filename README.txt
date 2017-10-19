# Scalable PaleoGeomorphology Model (SPGM)

## Description
SPGM embodies the implementation of a multi-process numerical model, where physical
processes that contribute to mass redistribution on earth's surface are integrated separately to record geomorphological evolution. We leverage recent advances in flow routing algorithms and present a self-contained, extensible software framework that comprises a collection of modules which describe physical processes influencing earth's surface topography through time. The modular structure of the code, with clearly defined interfaces between mesh generation, implementation of optimized numerical algorithms and output generation is particularly geared toward ease of adaptability for studying a wide range of geomorphological scenarios.


## Dependencies 

SPGM uses **scons** for its build-system. On Linux/Unix systems **scons** can be installed using respective package management systems. On Macs, **scons** can be installed through **fink** or **homebrew**. Apart from **scons**, SPGM is fully self-contained and has no other dependencies. The code has been tested to run on Linux and Mac OSX systems with a gcc compiler (version 4.7+) installed. It should, nonetheless, work with earlier but relatively recent versions of the gcc compiler.

## Compilation

The code can be compiled from the base directory as follows:
```sh
scons
```
Builds can be parallelized by passing in an optional falg, ```-j4```, which in this case will spawn 4 threads to speed up the build process, especially on multicore machines. The compiled executable can be found in ```build/release/program/spgm```, relative to the base directory.

A debug executable can be built as follows:
```sh
scons debug=1
```

The test-suite in SPGM can be run as follows:
```sh
scons test=1
```

A build can be cleaned as follows:
```sh
scons -c
```

## Running Examples
The simple models in the 'example' folder should be run from within the model folders by invoking the executable with the corresponding parameter file. For instance, the first example can be run as follows:

```sh
 cd <base-path>/examples/ex1
 ../../build/release/src/program/spgm ex1.cfg
```


## Copyright

Please see COPYING.txt for details.
