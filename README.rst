-   `1. Introduction`_

-   `1.1. Features`_

-   `2. New in this release`_

-   `2.1. New Features`_
-   `2.2. Enhancements`_
-   `2.3. Optimizations`_
-   `2.4. Examples`_
-   `2.5. Tools`_
-   `2.6. Documentation`_
-   `2.7. Bug Fixes`_
-   `2.8. Known Bugs`_

-   `3. About`_

-   `3.1. Operating System Support`_
-   `3.2. Window System Support`_
-   `3.3. Documentation`_
-   `3.4. Support`_

-   `4. Errata`_


1. Introduction
---------------

Welcome to Equalizer, the standard middleware to create and deploy parallel,
scalable OpenGL applications. This release introduces major new features,
most notably asynchronous readbacks, region of interest and thread affinity
for increased performance during scalable rendering.

Equalizer 1.4 is a feature release extending the 1.0 API, distilling over
seven years of development and decades of experience into a feature-rich,
high-performance and mature parallel rendering framework and an object-
oriented high-level network library. It is intended for all application
developers creating parallel, interactive OpenGL applications. Equalizer 1.4
can be retrieved by downloading the `source code`_ or one of the `precompiled
packages`_.


1.1. Features
~~~~~~~~~~~~~

Equalizer provides the following major features to facilitate the development
and deployment of scalable OpenGL applications. A `detailed feature list`_
can be found on the Equalizer website.

-   **Runtime Configurability:** An Equalizer application is configured
    automatically or manually at runtime and can be deployed on laptops,
    multi-GPU workstations and large-scale visualization clusters without
    recompilation.
-   **Runtime Scalability:** An Equalizer application can benefit from
    multiple graphics cards, processors and computers to scale rendering
    performance, visual quality and display size.
-   **Distributed Execution:** Equalizer applications can be written to
    support cluster-based execution. Equalizer furnishes and uses the Collage
    network library, a cross-platform C++ library for building heterogenous,
    distributed applications.
-   **Support for Stereo and Immersive Environments:** Equalizer supports
    stereo rendering head tracking, head-mounted displays and other advanced
    features for immersive Virtual Reality installations.


2. New in this release
----------------------

Equalizer 1.4 contains the following features, enhancements, bug fixes and
documentation changes:


2.1. New Features
~~~~~~~~~~~~~~~~~

-   `Asynchronous readback`_ support
-   `Automatic CPU-GPU affinity`_
-   `Application-specific scaling`_ to visualize data in a scale
    different to 1:1 in immersive environments
-   `VirtualGL-aware auto-configuration`_
-   `Region of interest`_ for scalable rendering and load-balancing

-   `Zeroconf support and node discovery`_
-   `Blocking co::Object::commit`_
-   `Extensible packet dispatch`_


2.2. Enhancements
~~~~~~~~~~~~~~~~~

-   `System window without drawable buffer`_
-   `Mac OS X: Build universal libraries even when AGL is enabled`_
-   auto-config: add direct send configuration
-   auto-config: save generated configuration to .eqc file
-   auto-config: application-specific flags for multiprocess execution


2.3. Optimizations
~~~~~~~~~~~~~~~~~~

-   `Multi-GPU NVidia optimization`_
-   load_equalizer: split along longest axis in 2D mode

-   InfiniBand RDMA: significant performance increase using a different
    underlying implementation


2.4. Examples
~~~~~~~~~~~~~

-   eqPly: Add command line option to disable region of interest
-   eqPly: Parallel kd-tree construction when using gcc 4.4 or later
-   eqPly: runtime-changeable model unit scaling
-   eqPly: Create all VBOs/display lists during the first frame


2.5. Tools
~~~~~~~~~~

-   eqPlyConverter: New offline tool to generate binary cache for eqPly


2.6. Documentation
~~~~~~~~~~~~~~~~~~

The following documentation has been added or substantially improved since
the last release:

-   Full `API documentation`_ for the public Equalizer API
-   The `Programming and User Guide`_ has been extended to 107 pages and
    60 figures
-   `Tile compounds`_ using a pull-based task distribution for volume
    rendering and interactive raytracing


2.7. Bug Fixes
~~~~~~~~~~~~~~

Equalizer 1.4 includes various bugfixes over the 1.2.1 release, including the
following:

-   `139`_: Tile compound readback broken
-   `120`_: Async readback deallocation
-   `118`_: OS X: Async readback doesn't work
-   `137`_: 1-window.DFR broken
-   `136`_: compositor assertion when using custom frames
-   `135`_: Command line option --eq-layout broken
-   `131`_: seqPly --help launches application
-   `127`_: Problem with getdomainname() in SocketConnection::listen()
-   `124`_: Upload plugins are not freed
-   `121`_: Packaging: netperf conflicts with other packages
-   `117`_: Race with async channel tasks


2.8. Known Bugs
~~~~~~~~~~~~~~~

The following bugs were known at release time. Please file a `Bug Report`_ if
you find any other issue with this release.

-   `138`_: Windows: PBO error when rendering
-   `78`_: AGL: assertion on interaction with multiple GPUs
-   `77`_: 7-window.DB.PIXEL.eqc broken
-   `76`_: 7-window.DPLEX.2D.lb.eqc does not load-balance
-   `49`_: eqPixelBench crash with double free
-   `19`_: zoom readback with FBO
-   `18`_: zoom: depth readback does not work
-   `17`_: AGL: Window close does not work


3. About
--------

Equalizer is a cross-platform toolkit, designed to run on any modern
operating system, including all Unix variants and the Windows operating
system. A `compatibility matrix`_ can be found on the Equalizer website.

Equalizer requires at least `OpenGL 1.1`_, but uses newer OpenGL features
when available. Version 1.4 has been tested on:


3.1. Operating System Support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Equalizer uses CMake to create a platform-specific build environment. The
following platforms and build environments are tested:

-   **Linux:** Ubuntu 11.10, 12.04, RHEL 6.1 (Makefile, i386, x64)
-   **Windows:** 7 (Visual Studio 2008, i386, x64)
-   **Mac OS X:** 10.7 (Makefile, XCode, i386, x64)


3.2. Window System Support
~~~~~~~~~~~~~~~~~~~~~~~~~~

-   **X11:** Full support for all documented features
-   **WGL:** Full support for all documented features
-   **AGL:** Full support for all documented features


3.3. Documentation
~~~~~~~~~~~~~~~~~~

The Programming and User Guide is available as a `hard-copy`_ and `online`_.
The `API documentation`_ can be found on the Equalizer website.

As with any open source project, the available source code, in particular the
shipped `examples`_ provide a reference for developing or porting
applications. The `Developer Documentation`_ on the website provides further
design documents for specific features. XCode users can download a
`Documentation Set`_.


3.4. Support
~~~~~~~~~~~~

Technical questions can be posted to the ` Developer Mailing List`_, or
directly to ` info@equalizergraphics.com`_.

Commercial support, custom software development and porting services are
available from `Eyescale`_. Please contact `info@eyescale.ch`_ for further
information.


4. Errata
---------

.. _1. Introduction: #introduction
.. _1.1. Features: #features
.. _2. New in this release: #new
.. _2.1. New Features: #newFeatures
.. _2.2. Enhancements: #enhancements
.. _2.3. Optimizations: #optimizations
.. _2.4. Examples: #examples
.. _2.5. Tools: #tools
.. _2.6. Documentation: #documentation
.. _2.7. Bug Fixes: #bugfixes
.. _2.8. Known Bugs: #knownbugs
.. _3. About: #about
.. _3.1. Operating System Support: #os
.. _3.2. Window System Support: #ws
.. _3.3. Documentation: #documentation
.. _3.4. Support: #support
.. _4. Errata: #errata
.. _source     code:
    http://www.equalizergraphics.com/downloads/Equalizer-1.3.6.tar.gz
.. _precompiled packages:
    http://www.equalizergraphics.com/downloads/developer.html
.. _detailed feature list: /features.html
.. _Asynchronous       readback:
    http://www.equalizergraphics.com/documents/design/asyncCompositing.html
.. _Automatic       CPU-GPU affinity:
    https://github.com/Eyescale/Equalizer/issues/57
.. _Application-specific       scaling:
    https://github.com/Eyescale/Equalizer/issues/63
.. _VirtualGL-aware       auto-configuration:
    https://github.com/Eyescale/Equalizer/issues/67
.. _Region       of interest:
    http://www.equalizergraphics.com/documents/design/roi.html
.. _Zeroconf       support and node discovery:
    https://github.com/Eyescale/Equalizer/issues/122
.. _Blocking       co::Object::commit:
    https://github.com/Eyescale/Equalizer/issues/116
.. _Extensible       packet dispatch:
    https://github.com/Eyescale/Equalizer/issues/111
.. _System window       without drawable buffer:
    https://github.com/Eyescale/Equalizer/issues/70
.. _Mac OS X: Build       universal libraries even when AGL is enabled:
    https://github.com/Eyescale/Equalizer/issues/123
.. _Multi-GPU NVidia       optimization:
    https://github.com/Eyescale/Equalizer/issues/95
.. _API       documentation:
    http://www.equalizergraphics.com/documents/Developer/API-1.4/index.html
.. _Programming and       User Guide:
    http://www.equalizergraphics.com/survey.html
.. _Tile compounds: /documents/design/tileCompounds.html
.. _139: https://github.com/Eyescale/Equalizer/issues/139
.. _120: https://github.com/Eyescale/Equalizer/issues/120
.. _118: https://github.com/Eyescale/Equalizer/issues/118
.. _137: https://github.com/Eyescale/Equalizer/issues/137
.. _136: https://github.com/Eyescale/Equalizer/issues/136
.. _135: https://github.com/Eyescale/Equalizer/issues/135
.. _131: https://github.com/Eyescale/Equalizer/issues/131
.. _127: https://github.com/Eyescale/Equalizer/issues/127
.. _124: https://github.com/Eyescale/Equalizer/issues/124
.. _121: https://github.com/Eyescale/Equalizer/issues/121
.. _117: https://github.com/Eyescale/Equalizer/issues/117
.. _Bug Report: https://github.com/Eyescale/Equalizer/issues
.. _138: https://github.com/Eyescale/Equalizer/issues/138
.. _78: https://github.com/Eyescale/Equalizer/issues/78
.. _77: https://github.com/Eyescale/Equalizer/issues/77
.. _76: https://github.com/Eyescale/Equalizer/issues/76
.. _49: https://github.com/Eyescale/Equalizer/issues/49
.. _19: https://github.com/Eyescale/Equalizer/issues/19
.. _18: https://github.com/Eyescale/Equalizer/issues/18
.. _17: https://github.com/Eyescale/Equalizer/issues/17
.. _compatibility   matrix:
    http://www.equalizergraphics.com/compatibility.html
.. _OpenGL 1.1: http://www.opengl.org
.. _hard-copy: https://www.createspace.com/3943261
.. _online: http://www.equalizergraphics.com/survey.html
.. _API     documentation:
    http://www.equalizergraphics.com/documents/Developer/API-1.4/index.html
.. _examples: https://github.com/Eyescale/Equalizer/tree/1.3.5/examples
.. _Developer Documentation:
    http://www.equalizergraphics.com/doc_developer.html
.. _Documentation     Set: http://www.equalizergraphics.com/documents/Dev
    eloper/API-1.4/ch.eyescale.Equalizer.docset.zip
.. _     Developer Mailing List: http://www.equalizergraphics.com/cgi-
    bin/mailman/listinfo/eq-dev
.. _     info@equalizergraphics.com:
    mailto:info@equalizergraphics.com?subject=Equalizer%20question
.. _Eyescale: http://www.eyescale.ch
.. _info@eyescale.ch: mailto:info@eyescale.ch?subject=Equalizer%20support
