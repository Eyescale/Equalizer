# Introduction

Welcome to Equalizer, the standard middleware to create and deploy parallel,
scalable OpenGL applications. It enables applications to benefit from multiple
graphics cards, processors and computers to scale the rendering performance,
visual quality and display size. An Equalizer application runs unmodified on any
visualization system, from a simple workstation to large scale graphics
clusters, multi-GPU workstations and Virtual Reality installations.

The Equalizer [Programming and User Guide](https://github.com/Eyescale/EqDocs/raw/master/Developer/ProgrammingGuide/paper.pdf) covers the basics of Equalizer
programming. The API documentation can be found on
[github](http://eyescale.github.com/).

As with any open source project, the available source code, in particular the
shipped examples provide a reference for developing or porting applications.

Technical questions can be posted to the eq-dev Mailing List, or
directly to info@equalizergraphics.com.

Commercial support, custom software development and porting services are
available from [Eyescale](http://www.eyescale.ch). Please contact
[info@eyescale.ch](mailto:info@eyescale.ch?subject=Equalizer%20support)
for further information.

# Features

Equalizer provides the following major features to facilitate the development
and deployment of scalable OpenGL applications. A
[detailed feature list](http://www.equalizergraphics.com/features.html) can be
found on the [Equalizer website](http://www.equalizergraphics.com). The
[change log](@ref Changelog) lists features, improvements and bug
fixes introduced in each version.

* Runtime Configurability: An Equalizer application is configured
  automatically or manually at runtime and can be deployed on laptops,
  multi-GPU workstations and large-scale visualization clusters without
  recompilation.
* Runtime Scalability: An Equalizer application can benefit from
  multiple graphics cards, processors and computers to scale rendering
  performance, visual quality and display size.
* Distributed Execution: Equalizer applications can be written to
  support cluster-based execution. Equalizer uses the
  [Collage network library](http://www.libcollage.net), a cross-platform
  C++ library for building heterogenous, distributed applications.
* Support for Stereo and Immersive Environments: Equalizer supports
  stereo rendering head tracking, head-mounted displays and other
  advanced features for immersive Virtual Reality installations.
* Detailed @ref Changelog


# Known Bugs

Please refer to the
[github issue tracker](https://github.com/Eyescale/Equalizer/issues) for
fixed and open bugs, and to report new bugs.

# Building from source

Equalizer is a cross-platform library, designed to run on any modern operating
system, including all Unix variants and the Windows operating system. Equalizer
requires at least [OpenGL 1.1](http://www.opengl.org), but uses newer OpenGL
features when available. Equalizer uses CMake to create a platform-specific
build environment. The following platforms and build environments are tested:

* Linux: Ubuntu 14.04, RHEL 6.6 (Makefile, Ninja)
* Windows: 7 (Visual Studio 2012)
* Mac OS X: 10.8 (Makefile, Ninja)

## Linux, Mac OS X

    git clone https://github.com/Eyescale/Equalizer.git
    mkdir Equalizer/build
    cd Equalizer/build
    cmake  -DINSTALL_PACKAGES=1 ..
    make

## Windows {#Windows}

If `CMake` and `git` are in PATH, run the following batch script to build using
Visual Studio 2013:

    build.bat

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
~~~
setx `BOOST_ROOT`                "F:\boost_1_57_0\"
setx `BOOST_INCLUDEDIR`          "F:\boost_1_57_0\"
setx `BOOST_LIBRARYDIR`          "F:\boost_1_57_0\lib32-msvc-12.0\"
setx `Boost_ADDITIONAL_VERSIONS` "1.57 1.57.0"
setx `Boost_USE_STATIC_LIBS`     "ON"
setx `Boost_USE_MULTITHREAD`     "ON"
~~~
