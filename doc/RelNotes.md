Release Notes {#mainpage}
============

[TOC]

# Introduction {#Introduction}

Welcome to Equalizer, the standard middleware to create and deploy
parallel, scalable OpenGL applications. This release introduces major
new features, most notably asynchronous readbacks, region of interest
and thread affinity for increased performance during scalable
rendering.

Equalizer 1.6 is a feature release extending the 1.0 API, distilling
over eight years of development and decades of experience into a
feature-rich, high-performance and mature parallel rendering
framework. It is intended for all application developers creating
parallel, interactive OpenGL applications. Equalizer 1.6 can be
retrieved by downloading the [source code](http://www.equalizergraphics.com/downloads/Equalizer-1.6.0.tar.gz") or one of the [precompiled packages](http://www.equalizergraphics.com/downloads/major.html).

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

Equalizer 1.6 contains the following features, enhancements, bug fixes
and documentation changes over the Equalizer 1.4 release:

## New Features {#NewFeatures}

* [204:](https://github.com/Eyescale/Equalizer/issues/204) Camera-based
  head tracking using OpenCV
* [196:](https://github.com/Eyescale/Equalizer/issues/196) Asymmetric
  eye position
* [194:](https://github.com/Eyescale/Equalizer/issues/194)
  Runtime-Changeable Segment Frusta
* [189:](https://github.com/Eyescale/Equalizer/issues/189) Runtime
  changeable equalizer properties

## Enhancements {#Enhancements}

* [186:](https://github.com/Eyescale/Equalizer/issues/186) Resistance to
  load_equalizers changes
* [185:](https://github.com/Eyescale/Equalizer/issues/185) Network
  interface selection for autoconfiguration
* [153:](https://github.com/Eyescale/Equalizer/issues/153) GLEW 1.9.0 update
* [150:](https://github.com/Eyescale/Equalizer/issues/150) Spacemouse
  Linux support

## Optimizations {#Optimizations}

*

## Tools {#Tools}

* New coNodePerf application to benchmark node-to-node messaging performance

## Documentation {#Documentation}

The following documentation has been added or substantially improved
since the last release:

* Full documentation for the public Collage API
* Expanded Collage content in the Equalizer Programming and User Guide

## Bug Fixes {#Fixes}

Collage 1.0 includes various bugfixes over the Equalizer 1.4 release,
including the following:

* [22](https://github.com/Eyescale/Collage/issues/22) Crash during
  concurrent object deregister and map
* [13](https://github.com/Eyescale/Collage/issues/13): Global argument
  parsing broken

## Known Bugs {#Bugs}

The following bugs were known at release time. Please file a [Bug Report](https://github.com/Eyescale/Collage/issues) if you find any other issue with this release.

* [16](https://github.com/Eyescale/Collage/issues/16): RSP Interface not
  bound on Linux
* [15](https://github.com/Eyescale/Collage/issues/15): RDMAConnection
  not endian-safe
* [14](https://github.com/Eyescale/Collage/issues/14): coNetperf server
  occasionally crashes on client disconnect
* [3](https://github.com/Eyescale/Collage/issues/3): Snappy compressor
  does not work on PPC
* [2](https://github.com/Eyescale/Collage/issues/2): Multiple dispatcher
  inheritance not working with xlC

# About {#About}

Collage is a cross-platform library, designed to run on any modern
operating system, including all Unix variants and the Windows operating
system. Collage uses CMake and
[Buildyard](https://github.com/Eyescale/Buildyard) to create a
platform-specific build environment. The following platforms and build
environments are tested for version 1.0:

* Linux: Ubuntu 12.04, 12.10, RHEL 6.3 (Makefile, i386, x64)
* Windows: 7 (Visual Studio 2008, i386, x64)
* Mac OS X: 10.8 (Makefile, XCode, i386, x64)

The Equalizer Programming and User Guide covers the basics of Collage
programming. The API documentation can be found on
[github](http://eyescale.github.com/).

As with any open source project, the available source code, in
particular the shipped tools provide a reference for developing or
porting applications.

Technical questions can be posted to the eq-dev Mailing List, or
directly to info@equalizergraphics.com.

Commercial support, custom software development and porting services are
available from [Eyescale](http://www.eyescale.ch). Please contact
[info@eyescale.ch](mailto:info@eyescale.ch?subject=Collage%20support)
for further information.

# Errata
