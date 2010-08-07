
Equalizer now can be built using CMake. A few glitches still remain, but I hope we can deprecate
all other build systems ASAP.

What works (tested):
 * building and installing on Ubuntu and Windows
 * creating binary package for Ubuntu
 * creating binary installer for Windows (with selectable components!)
 * tests are run by CTest (some fail however)

Not tested (maybe works):
 * examples/osgScaleViewer fails to compile (I tested with an outdated v2.8.1)


Ubuntu 32bit + 64bit, gcc 4.4.3
===============================
$ mkdir build
$ cd build
$ cmake ..
$ make


Windows XP 32bit using Visual C++ 2008 Express
==============================================
Use CMake GUI
 1. choose source dir and build dir
 2. hit 'Configure' a few times, resolve red entries, hit 'Configure' again
 3. hit 'Generate' once available
 4. open the generated solution with visual studio

Known Problems:
 * Bison in Windows/bin does not work for whatever reason (probably outdated)
     1. Download Bison and Flex and install them in a directory that does not contain spaces
        NOTE: The default setting "C:\Program Files\GnuWin32\" will NOT work!!
     2. add the bin directory to the PATH environment variable (otherwise bison fails to launch m4)
     3. Configure the path of bison and flex in the CMake GUI


Ubuntu 32bit, Crosscompiling with gcc-mingw32 44.2
===============================
$ mkdir build
$ cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake ..
$ make

Known Problems:
  * Two files fail to compile
     -> Someone knowledgeable about the source will need to have a look
