
Equalizer is built using CMake. Please report any issues to the eq-dev
mailing list.

Tested are:
 * building and installing on Ubuntu, Mac OS X and Windows
 * creating binary package for Ubuntu
 * creating binary installer for Windows (with selectable components!)
 * creating binary installer for Mac OS X
 * tests are run by CTest (some fail however)


Posix platforms (Linux, Mac OS X)
===============================

$ make (debug|release(|xcode))

You may also use CMake directly. The default Makefile exist purely for
convenience.

Windows, Visual Studio
======================
Use CMake GUI
 1. choose source dir and build dir
 2. hit 'Configure' a few times, resolve red entries, hit 'Configure' again
 3. hit 'Generate' once available
 4. open the generated solution with visual studio


Ubuntu 32bit, Crosscompiling with gcc-mingw32 44.2
===============================
$ mkdir build
$ cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake ..
$ make
