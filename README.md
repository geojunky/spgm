# Scalable PaleoGeomorphology Model (SPGM)

![alt text](https://travis-ci.org/rh-downunder/spgm.svg?branch=master)

## Description
SPGM embodies the implementation of a multi-process numerical model, where physical
processes that contribute to mass redistribution on earth's surface are integrated separately to record geomorphological evolution. We leverage recent advances in flow routing algorithms and present a self-contained, extensible software framework that comprises a collection of modules which describe physical processes influencing earth's surface topography through time. The modular structure of the code, with clearly defined interfaces between mesh generation, implementation of optimized numerical algorithms and output generation is particularly geared toward ease of adaptability for studying a wide range of geomorphological scenarios.


## Dependencies 

SPGM uses **scons** for its build-system. Apart from **scons**, SPGM is fully self-contained and has no other dependencies. The code has been tested to run on Linux/Unix, Cygwin and Mac OS systems with a gcc compiler (version 4.7+) installed. It should, nonetheless, work with earlier but relatively recent versions of the gcc compiler.

###Linux (Ubuntu)
1. Install **scons** using ```apt-get```
```sh
apt-get install scons
```

###Cygwin
1. The Cygwin ```setup``` program can be used to install **scons**

###OSX
1. Install **scons** using **MacPorts**:
  ```sh
sudo port install scons
  ```
Alternatively, Install **scons** using **homebrew**:
  ```sh
brew install scons
  ```

2. Set ```gcc``` compiler version :
	```bash
	export CXX=g++-7
	```
	Note, ```g++-7``` serves only as an example -- ```CXX``` should be set to the gcc compiler installed on the system.

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
