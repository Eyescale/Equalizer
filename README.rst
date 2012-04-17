-   `1. Introduction`_

-   `1.1. Features`_

-   `2. New in this release`_

-   `2.1. New Features`_
-   `2.2. Enhancements`_
-   `2.3. Optimizations`_
-   `2.4. Examples`_
-   `2.5. Tools`_
-   `2.6. API Changes`_
-   `2.7. Documentation`_
-   `2.8. Bug Fixes`_
-   `2.9. Known Bugs`_

-   `3. About`_

-   `3.1. Operating System Support`_
-   `3.2. Window System Support`_
-   `3.3. Documentation`_
-   `3.4. Support`_

-   `4. Errata`_


1. Introduction
---------------

Welcome to Equalizer, the standard middleware to create and deploy parallel,
scalable OpenGL applications. This release is a bugfix release for Equalizer
1.2.

Equalizer 1.2 culminates over seven years of development and decades of
experience into a feature-rich, high-performance and mature parallel
rendering framework and an object-oriented high-level network library. It is
intended for all application developers creating parallel, interactive OpenGL
applications. Equalizer 1.2.1 can be retrieved by downloading the `source
code`_ or one of the `precompiled packages`_.


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

Equalizer 1.2.1 contains the following features, enhancements, bug fixes and
documentation changes:


2.1. New Features
~~~~~~~~~~~~~~~~~

No new features


2.2. Enhancements
~~~~~~~~~~~~~~~~~

No enhancements


2.3. Optimizations
~~~~~~~~~~~~~~~~~~

-   `109`_: Race and performance issue with multicast object distribution


2.4. Examples
~~~~~~~~~~~~~

No example changes


2.5. Tools
~~~~~~~~~~

No tool changes


2.6. API Changes
~~~~~~~~~~~~~~~~

No API changes


2.7. Documentation
~~~~~~~~~~~~~~~~~~

No documentation changes


2.8. Bug Fixes
~~~~~~~~~~~~~~

Equalizer 1.2.1 includes the following bugfixes over the 1.2 release:

-   `103`_: RSP race during connection
-   `105`_: Sequel missing from packages
-   `107`_: Installed examples CMake build does not work
-   `112`_: --eq-layout gets activated on second frame


2.9. Known Bugs
~~~~~~~~~~~~~~~

The following bugs were known at release time. Please file a `Bug Report`_ if
you find any other issue with this release.

-   `78`_: AGL: assertion on interaction with multiple GPUs
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
when available. Version 1.2 has been tested on:


3.1. Operating System Support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Equalizer uses CMake to create a platform-specific build environment. The
following platforms and build environments are tested:

-   **Linux:** Ubuntu 11.04, 11.10, RHEL 6.1 (Makefile, i386, x64)
-   **Windows:** XP and 7 (Visual Studio 2008, i386, x64)
-   **Mac OS X:** 10.6, 10.7 (Makefile, XCode, i386, x64)


3.2. Window System Support
~~~~~~~~~~~~~~~~~~~~~~~~~~

-   **X11:** Full support for all documented features.
-   **WGL:** Full support for all documented features.
-   **AGL:** Full support for all documented features.


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
.. _2.6. API Changes: #changes
.. _2.7. Documentation: #documentation
.. _2.8. Bug Fixes: #bugfixes
.. _2.9. Known Bugs: #knownbugs
.. _3. About: #about
.. _3.1. Operating System Support: #os
.. _3.2. Window System Support: #ws
.. _3.3. Documentation: #documentation
.. _3.4. Support: #support
.. _4. Errata: #errata
.. _source     code:
    http://www.equalizergraphics.com/downloads/Equalizer-1.2.1.tar.gz
.. _precompiled packages:
    http://www.equalizergraphics.com/downloads/major.html#1.2
.. _detailed feature list: /features.html
.. _109: https://github.com/Eyescale/Equalizer/issues/109
.. _103: https://github.com/Eyescale/Equalizer/issues/103
.. _105: https://github.com/Eyescale/Equalizer/issues/105
.. _107: https://github.com/Eyescale/Equalizer/issues/107
.. _112: https://github.com/Eyescale/Equalizer/issues/112
.. _Bug Report: https://github.com/Eyescale/Equalizer/issues
.. _78: https://github.com/Eyescale/Equalizer/issues/78
.. _76: https://github.com/Eyescale/Equalizer/issues/76
.. _49: https://github.com/Eyescale/Equalizer/issues/49
.. _19: https://github.com/Eyescale/Equalizer/issues/19
.. _18: https://github.com/Eyescale/Equalizer/issues/18
.. _17: https://github.com/Eyescale/Equalizer/issues/17
.. _compatibility   matrix:
    http://www.equalizergraphics.com/compatibility.html
.. _OpenGL 1.1: http://www.opengl.org
.. _hard-copy: https://www.createspace.com/3800793
.. _online: http://www.equalizergraphics.com/survey.html
.. _API     documentation:
    http://www.equalizergraphics.com/documents/Developer/API-1.2/index.html
.. _examples: https://github.com/Eyescale/Equalizer/tree/1.2.1/examples
.. _Developer Documentation:
    http://www.equalizergraphics.com/doc_developer.html
.. _Documentation     Set: http://www.equalizergraphics.com/documents/Dev
    eloper/API-1.2/ch.eyescale.Equalizer.docset.zip
.. _     Developer Mailing List: http://www.equalizergraphics.com/cgi-
    bin/mailman/listinfo/eq-dev
.. _     info@equalizergraphics.com:
    mailto:info@equalizergraphics.com?subject=Equalizer%20question
.. _Eyescale: http://www.eyescale.ch
.. _info@eyescale.ch: mailto:info@eyescale.ch?subject=Equalizer%20support
