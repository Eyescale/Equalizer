# Equalizer Parallel Rendering Framework

[![Join the chat at https://gitter.im/Eyescale/Equalizer](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Eyescale/Equalizer?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Welcome to Equalizer, the standard middleware to create and deploy parallel,
scalable OpenGL applications. It enables applications to benefit from multiple
graphics cards, processors and computers to scale the rendering performance,
visual quality and display size. An Equalizer application runs unmodified on any
visualization system, from a simple workstation to large scale graphics
clusters, multi-GPU workstations and Virtual Reality installations.

# Building from source
## Linux, Mac OS X
```
  mkdir build
  cd build
  cmake ..
  make
```
## Windows
If `CMake` and `git` are in PATH, run the following batch script to build using
Visual Studio 2013:
```
  build.bat
```
## A note about BOOST + CMake on Windows
If your build fails with a `Could NOT find Boost` message, you may have a bad
environment setup for `Boost` and `CMake`. To remedy this, `CMake` requires you
to properly hint it to find your `Boost` distribution. Make sure you have the
following variables set up:

 1. `BOOST_ROOT` pointing to your Boost root directory.
 2. `BOOST_INCLUDEDIR` pointing to your Boost includes (usually where
    `BOOST_ROOT` points to).
 3. `BOOST_LIBRARYDIR` pointing to your Boost binary directory
    (`lib32-msvc-12.0` for instance).
 4. `Boost_ADDITIONAL_VERSIONS` having major, minor, and patch versions
    (separated by space).
 5. *(optional)* `Boost_USE_STATIC_LIBS` set to `ON` if you want to statically
    link to `Boost`. (default is `OFF`)
 6. *(optional)* `Boost_USE_MULTITHREAD` set to `ON` if you want to use `Boost`
    multi-threaded libraries. (default is `ON`)

An example set up would be (typed in Windows command prompt):
```
setx `BOOST_ROOT`                "F:\boost_1_57_0\"
setx `BOOST_INCLUDEDIR`          "F:\boost_1_57_0\"
setx `BOOST_LIBRARYDIR`          "F:\boost_1_57_0\lib32-msvc-12.0\"
setx `Boost_ADDITIONAL_VERSIONS` "1.57 1.57.0"
setx `Boost_USE_STATIC_LIBS`     "ON"
setx `Boost_USE_MULTITHREAD`     "ON"
```
