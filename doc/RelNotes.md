Release Notes {#mainpage}
============

[TOC]

# Introduction {#Introduction}

Welcome to Equalizer, the standard middleware to create and deploy
parallel, scalable OpenGL applications. This release introduces major
new features, most notably integrated head tracking using VRPN or OpenCV.

Equalizer 1.8 is a feature release extending the 1.0 API, distilling
over several years of development and decades of experience into a
feature-rich, high-performance and mature parallel rendering
framework. It is intended for all application developers creating
parallel, interactive OpenGL applications. Equalizer 1.8 can be
retrieved by downloading the [source code](http://www.equalizergraphics.com/downloads/Equalizer-1.8.0.tar.gz") or one of the [precompiled packages](http://www.equalizergraphics.com/downloads/major.html).

## Features {#Features}

Equalizer provides the following major features to facilitate the development and deployment of scalable OpenGL applications. A [detailed feature list](http://www.equalizergraphics.com/features.html) can be found on the [Equalizer website](http://www.equalizergraphics.com).

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

# Changes {#Changes}

## git master

* Implemented eq::x11::Window and eqCPU example as a template for
  CPU-based rendering, for example for raytracing

## Equalizer 1.8

* Implemented Qt window system for onscreen windows. Issue
  [21](https://github.com/Eyescale/Equalizer/issues/21) partially done.
* DisplayCluster streaming can be enabled with automatic configuration using new
  global view attributes: EQ_VIEW_SATTR_DISPLAYCLUSTER and
  EQ_VIEW_SATTR_PIXELSTREAM_NAME.
* New EqGLLibraries.cmake script for simpler OpenGL configuration in
  downstream projects
* Compression is enabled for DisplayCluster streaming
* DisplayCluster streaming is now asynchronous

## Bugs {#Bugs}

Please refer to the
[github issue tracker](https://github.com/Eyescale/Equalizer/issues) for
fixed and open bugs, and to report new bugs.

# About {#About}

Equalizer is a cross-platform framework, designed to run on any modern
operating system, including all Unix variants and the Windows operating
system. Equalizer requires at least [OpenGL 1.1](http://www.opengl.org),
but uses newer OpenGL features when available. Equalizer uses CMake and
[Buildyard](https://github.com/Eyescale/Buildyard) to create a
platform-specific build environment. The following platforms and build
environments are tested for version 1.8:

* Linux: Ubuntu 14.04, RHEL 6.5 (Makefile, i386, x64)
* Windows: 7 (Visual Studio 2008, i386, x64)
* Mac OS X: 10.8 (Makefile, XCode, i386, x64)

The Equalizer Programming and User Guide covers the basics of Collage
programming. The API documentation can be found on
[github](http://eyescale.github.com/).

As with any open source project, the available source code, in
particular the shipped examples provide a reference for developing or
porting applications.

Technical questions can be posted to the eq-dev Mailing List, or
directly to info@equalizergraphics.com.

Commercial support, custom software development and porting services are
available from [Eyescale](http://www.eyescale.ch). Please contact
[info@eyescale.ch](mailto:info@eyescale.ch?subject=Collage%20support)
for further information.

# Errata
