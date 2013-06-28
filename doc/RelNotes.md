Release Notes {#mainpage}
============

[TOC]

# Introduction {#Introduction}

Welcome to Equalizer, the standard middleware to create and deploy
parallel, scalable OpenGL applications. This release introduces major
new features, most notably integrated head tracking using VRPN or OpenCV.

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

* [204](https://github.com/Eyescale/Equalizer/issues/204): Camera-based
  head tracking using OpenCV
* [196](https://github.com/Eyescale/Equalizer/issues/196): Asymmetric
  eye position
* [194](https://github.com/Eyescale/Equalizer/issues/194): Runtime
  changeable segment frusta
* [189](https://github.com/Eyescale/Equalizer/issues/189): Runtime
  changeable equalizer properties
* [133](https://github.com/Eyescale/Equalizer/issues/133): VRPN head tracking

## Enhancements {#Enhancements}

* [186](https://github.com/Eyescale/Equalizer/issues/186): Resistance to
  load_equalizers changes
* [185](https://github.com/Eyescale/Equalizer/issues/185): Network
  interface selection for autoconfiguration
* [153](https://github.com/Eyescale/Equalizer/issues/153): GLEW 1.9.0 update
* [150](https://github.com/Eyescale/Equalizer/issues/150): Spacemouse
  Linux support
* [110](https://github.com/Eyescale/Equalizer/issues/110): Refactored
  statistics handling into external GLStats library
* [166](https://github.com/Eyescale/Equalizer/issues/166): Custom HMD projection
* [222](https://github.com/Eyescale/Equalizer/issues/222): Add default
  frustum on canvas

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

Equalizer 1.6 includes various bugfixes over the 1.4 release, including
the following:

* [138](https://github.com/Eyescale/Equalizer/issues/138),
  [174](https://github.com/Eyescale/Equalizer/issues/174),
  [177](https://github.com/Eyescale/Equalizer/issues/177),
  [183](https://github.com/Eyescale/Equalizer/issues/183): Shared
  rendering context setup bugs
* [192](https://github.com/Eyescale/Equalizer/issues/192):
  Auto-configured server connections are not removed
* [203](https://github.com/Eyescale/Equalizer/issues/203): Sequel: crash
  on failed window initialization
* [201](https://github.com/Eyescale/Equalizer/issues/201): util::Accum
  broken for operations with offset
* [217](https://github.com/Eyescale/Equalizer/issues/217): Auto image
  compressor allocation always uses lossy downloader data
* [76](https://github.com/Eyescale/Equalizer/issues/76):
  7-window.DPLEX.2D.lb.eqc does not load-balance
* [187](https://github.com/Eyescale/Equalizer/issues/187): TileEqualizer
  queue linking broken
* [199](https://github.com/Eyescale/Equalizer/issues/199): Alpha value
  wrong in eq::util::Accum::display
* [233](https://github.com/Eyescale/Equalizer/issues/233): Monitor
  equalizer with FBO sources leads to crash

## Known Bugs {#Bugs}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/Eyescale/Equalizer/issues) if you find
any other issue with this release.

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
environments are tested for version 1.6:

* Linux: Ubuntu 12.04, 12.10, RHEL 6.4 (Makefile, i386, x64)
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
