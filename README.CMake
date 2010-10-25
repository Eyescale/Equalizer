
Equalizer now can be built using CMake. A few glitches still remain, but
I hope we can deprecate all other build systems ASAP.

What works (tested):
 * building and installing on Ubuntu, Mac OS X and Windows
 * creating binary package for Ubuntu
 * creating binary installer for Windows (with selectable components!)
 * creating binary installer for Mac OS X
 * tests are run by CTest (some fail however)


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


Ubuntu 32bit, Crosscompiling with gcc-mingw32 44.2
===============================
$ mkdir build
$ cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake ..
$ make

Known Problems:
  * Two files fail to compile
     -> Someone knowledgeable about the source will need to have a look
