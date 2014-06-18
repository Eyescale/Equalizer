Release Notes {#mainpage}
============

[TOC]

# Introduction {#Introduction}

Welcome to Equalizer, the standard middleware to create and deploy
parallel, scalable OpenGL applications. This release introduces major
new features, most notably integrated head tracking using VRPN or OpenCV.

Equalizer 1.7 is a feature release extending the 1.0 API, distilling
over several years of development and decades of experience into a
feature-rich, high-performance and mature parallel rendering
framework. It is intended for all application developers creating
parallel, interactive OpenGL applications. Equalizer 1.7 can be
retrieved by downloading the [source code](http://www.equalizergraphics.com/downloads/Equalizer-1.7.0.tar.gz") or one of the [precompiled packages](http://www.equalizergraphics.com/downloads/major.html).

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

# New in this release {#New}

Equalizer 1.7 contains the following features, enhancements, bug fixes
and documentation changes over the Equalizer 1.6 release:

## New Features {#NewFeatures}

* [267](https://github.com/Eyescale/Equalizer/issues/267): Image dumping
  features added
* [280](https://github.com/Eyescale/Equalizer/issues/280): Sequel: change
  the near and far planes
* [299](https://github.com/Eyescale/Equalizer/issues/299): Added two new
  window attributes to set the width and height

## Enhancements {#Enhancements}

* [264](https://github.com/Eyescale/Equalizer/issues/264): Move away from
  removed/deprecated lunchbox API
* [269](https://github.com/Eyescale/Equalizer/issues/269): Generalize test
  config and put more tokens in it
* [272](https://github.com/Eyescale/Equalizer/issues/272): Classes related
  to PLY models moved to its own library
* [285](https://github.com/Eyescale/Equalizer/issues/285): Added global
  parameters to global section of the config: tcp_recv_buffer_size,
  tcp_send_buffer_size, read_thread_count
* [289](https://github.com/Eyescale/Equalizer/issues/289): Install pdb files
  for windows examples in debug builds
* [290](https://github.com/Eyescale/Equalizer/issues/290): Updated Equalizer
  to the new dc::Stream API.
* [308](https://github.com/Eyescale/Equalizer/issues/308): Remove unnecessary
  null pointer checks
* [331](https://github.com/Eyescale/Equalizer/issues/331): Add syncObject
  to ObjectHandler

## Optimizations {#Optimizations}

* No optimizations

## Examples {#Examples}

* eqPly: removed serial Flock of Birds tracker support, superseeded by
  OpenCV and VRPN tracking support

## Documentation {#Documentation}

The following documentation has been added or substantially improved
since the last release:

* Full documentation for the public Equalizer API
* Expanded content in the Equalizer Programming and User Guide

## Bug Fixes {#Fixes}

Equalizer 1.7 includes various bugfixes over the 1.6 release, including
the following:

* [291](https://github.com/Eyescale/Equalizer/issues/291): Document
  non-stack usage by protecting dtors
* [292](https://github.com/Eyescale/Equalizer/issues/292): FMAX/FMIN
  error on VS2013
* [302](https://github.com/Eyescale/Equalizer/issues/302): Not clamping
  window size hints to physical display if offscreen.
* [303](https://github.com/Eyescale/Equalizer/issues/303): Boost
  dependencies
* [309](https://github.com/Eyescale/Equalizer/issues/309): Build
  against new lunchbox and upcoming collage change
* [323](https://github.com/Eyescale/Equalizer/issues/323): 'Handle' failing
  frame set ready
* [324](https://github.com/Eyescale/Equalizer/issues/324): Admin
  window creation in eqPly
* [327](https://github.com/Eyescale/Equalizer/issues/327): Context
  sharing
* [328](https://github.com/Eyescale/Equalizer/issues/328): Remove
  Channel::getInput/OutputFrames
* [330](https://github.com/Eyescale/Equalizer/issues/330): Readback
  and assemble frames, replace deprecated ICommand funcs
* [332](https://github.com/Eyescale/Equalizer/issues/332): Output of
  readback types

## Known Bugs {#Bugs}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/Eyescale/Equalizer/issues) if you find
any other issue with this release.

* [232](https://github.com/Eyescale/Equalizer/issues/232): ROI with
  monitor equalizer results in wrong compositing
* [226](https://github.com/Eyescale/Equalizer/issues/226): Crash with
  layout switch on multi GPU system
* [167](https://github.com/Eyescale/Equalizer/issues/167): HWLOC:
  Segmentation Fault with empty auto thread affinity mask
* [77](https://github.com/Eyescale/Equalizer/issues/77):
  7-window.DB.PIXEL.eqc broken
* [78](https://github.com/Eyescale/Equalizer/issues/78): AGL: assertion
  on interaction with multiple GPUs
* [17](https://github.com/Eyescale/Equalizer/issues/17): AGL: Window
  close does not work
* [19](https://github.com/Eyescale/Equalizer/issues/19): zoom readback with FBO
* [18](https://github.com/Eyescale/Equalizer/issues/18): zoom: depth
  readback does not work

# About {#About}

Equalizer is a cross-platform framework, designed to run on any modern
operating system, including all Unix variants and the Windows operating
system. Equalizer requires at least [OpenGL 1.1](http://www.opengl.org),
but uses newer OpenGL features when available. Equalizer uses CMake and
[Buildyard](https://github.com/Eyescale/Buildyard) to create a
platform-specific build environment. The following platforms and build
environments are tested for version 1.7:

* Linux: Ubuntu 13.10, 14.04, RHEL 6.5 (Makefile, i386, x64)
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
* [236](https://github.com/Eyescale/Equalizer/issues/236): Bug in
  Equalizer.pc pkg-config file
* [0f5afef](https://github.com/Eyescale/Equalizer/commit/0f5afef) Fix
  PackageConfig generation to locate FindGLEW_MX
