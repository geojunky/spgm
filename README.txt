* ===========================================
*  Scalable PaleoGeomorphology Model (SPGM)
* ===========================================


Description
===========

SPGM embodies the implementation of a multi-process numerical model, where physical 
processes that contribute to mass redistribution are integrated separately to record 
geomorphological evolution. We leverage recent advances in flow routing algorithms 
and present a self-contained, extensible software framework that comprises a 
collection of modules that each describe a physical process influencing surface 
topography through time. The modular structure of the code, with clearly defined 
interfaces between mesh generation, implementation of optimized numerical algorithms 
and output generation is particularly geared toward ease of adaptability for studying 
a wide range of geomorphological scenarios.


Installation & Testing
======================

For compiling SPGM please see INSTALL.txt. The code can be tested by running the 
simple models in the 'examples' folder. These models should be run from within the 
model folder by invoking the executable with the corresponding parameter file. For 
instance, the first example can be run as follows:

# cd <base directory path>examples/ex1

# <base directory path>/build/release/program/spgm ex1.cfg


Copyright
=========

Please see COPYING.txt for details.
