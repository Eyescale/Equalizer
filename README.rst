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
scalable OpenGL applications. This release TBD

Equalizer 1.2 is a feature release extending the 1.0 API, distilling over 6
years of development and decades of experience into a feature-rich, high-
performance and mature parallel rendering framework and an object-oriented
high-level network library. It is intended for all application developers
creating parallel, interactive OpenGL applications. Equalizer 1.2 can be
retrieved by downloading the `source code`_ or one of the `precompiled
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
    both active and passive stereo rendering, as well as head tracking and
    head-mounted displays used in immersive Virtual Reality installations.


2. New in this release
----------------------

Equalizer 1.2 contains the following features, enhancements, bug fixes and
documentation changes:


2.1. New Features
~~~~~~~~~~~~~~~~~

-   `Automic local and remote configuration`_ using the `GPU-SD library
-   Initial release of `Sequel`_, a simplification and utility layer on
    top of Equalizer, enabling rapid development of clustered multi-GPU
    applications
-   Runtime failure tolerance detecting hardware and software failures
-   Tile compounds for fill-limited rendering such as volume raycasting
    and interactive raytracing

-   Distributed queue
-   RDMA-based connection class for InfiniBand
-   Support push-based object distribution


2.2. Enhancements
~~~~~~~~~~~~~~~~~

-   Added FindEqualizer.cmake and FindCollage.cmake for integration of
    Equalizer and Collage in CMake build environments
-   Implement EQ_WINDOW_IATTR_HINT_SWAPSYNC for GLX
-   Add time member to eq::Event recording time when the event was
    received from the operating system
-   Support for render clients without listening sockets
-   Allow the image compressor to be chosen by the application
-   Allow and prefer external GLEW installation during compilation
-   Updgrade internal GLEW version to 1.7.0
-   Per-segment or canvas swap barriers
-   `43`_: Add View::isActive and Layout::isActive
-   `45`_: Make RNG functional without co::base::init
-   Implement maximum size of multi-threaded queue, resulting in blocking
    push operations
-   Extend co::base::SpinLock and ScopedMutex with read-write semantics
-   Make Collage usable from multiple libraries by allowing init and exit
    to be called multiple times


2.3. Optimizations
~~~~~~~~~~~~~~~~~~

-   Make LocalNode::registerObject and Object::commit parallelizable by
    executing Object serialization from calling thread


2.4. Examples
~~~~~~~~~~~~~

-   Provide CMake files for installed examples
-   eqPly:
-   seqPly: An new example similar to eqPly, but using the Sequel API
-   eqAsync: A new example demonstrating OpenGL context sharing for
    asynchronously texture uploads
-   eqHello: Ported to Sequel


2.5. Tools
~~~~~~~~~~

-   No Changes


2.6. API Changes
~~~~~~~~~~~~~~~~

The following API changes may impact existing applications:

-   Removed co::Object::commitNB and commitSync since the commit request
    is no longer dispatched to command thread. Use commit instead.
-   Moved installed client headers to eq/client. Applications should
    always use eq/eq.h instead of individual headers.
-   Added a return value and timeout to
    co::LocalNode::acquireSendToken(), see method documentation.
-   Changed 'uint32_t eq::Version::getRevision()' to 'std::string
    eq::Version::getRevision()'


2.7. Documentation
~~~~~~~~~~~~~~~~~~

The following documentation has been added or substantially improved since
the last release:

-   Full `API documentation`_ for the public Equalizer API.
-   The `Programming and User Guide`_ has been extended to TBD pages and
    TBD figures.


2.8. Bug Fixes
~~~~~~~~~~~~~~

Equalizer 1.2 includes various bugfixes over the 1.0 release, including the
following:

-   RSP: Fix scattered ack implementation
-   `29`_: NV swap barrier with affinity context does not work
-   Fixed activation of OpenMP during build
-   Fixed glxew.h include
-   `3383573`_: Win32: occasional crash with CO_AGGRESSIVE_CACHING OFF
-   `3357684`_: View user data occasionally synced one frame too late
-   `3306308`_: Deadlock with config.eqc and window init failure
-   `3306148`_: Including eq/gl.h with AGL fails
-   `3301423`_: Config::stopFrames called from non-main thread causes
    deadlocks


2.9. Known Bugs
~~~~~~~~~~~~~~~

The following bugs were known at release time. Please file a `Bug Report`_ if
you find any other issue with this release.

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

-   **Linux:** Ubuntu 11.04, RHEL 6.1 (Makefile, i386, x64)
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
`API documentation`_ can be found on the Equalizer website.

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
    http://www.equalizergraphics.com/downloads/Equalizer-1.2.tar.gz
.. _precompiled packages:
    http://www.equalizergraphics.com/downloads/major.html#1.2
.. _detailed feature list: /features.html
.. _Automic       local and remote configuration: http://www.equalizergra
    phics.com/build/documentation/user/configuration.html
.. _Sequel: http://www.equalizergraphics.com//documents/Developer/API-1.2
    /sequel/namespaceseq.html
.. _43: https://github.com/Eyescale/Equalizer/issues/43
.. _45: https://github.com/Eyescale/Equalizer/issues/45
.. _API       documentation:
    http://www.equalizergraphics.com/documents/Developer/API-1.2/index.html
.. _Programming and       User Guide:
    http://www.equalizergraphics.com/survey.html
.. _29: https://github.com/Eyescale/Equalizer/issues/29
.. _3383573: https://sourceforge.net/tracker/?func=detail&aid=3383573&gro
    up_id=170962&atid=856209
.. _3357684: https://sourceforge.net/tracker/?func=detail&aid=3357684&gro
    up_id=170962&atid=856209
.. _3306308: https://sourceforge.net/tracker/?func=detail&aid=3306308&gro
    up_id=170962&atid=856209
.. _3306148: https://sourceforge.net/tracker/?func=detail&aid=3306148&gro
    up_id=170962&atid=856209
.. _3301423: https://sourceforge.net/tracker/?func=detail&aid=3301423&gro
    up_id=170962&atid=856209
.. _Bug Report: https://github.com/Eyescale/Equalizer/issues
.. _19: https://github.com/Eyescale/Equalizer/issues/19
.. _18: https://github.com/Eyescale/Equalizer/issues/18
.. _17: https://github.com/Eyescale/Equalizer/issues/17
.. _compatibility matrix:
    http://www.equalizergraphics.com/compatibility.html
.. _OpenGL 1.1: http://www.opengl.org
.. _hard-copy: http://www.lulu.com/product/paperback/equalizer-10
    -programming-and-user-guide/15165632
.. _online: http://www.equalizergraphics.com/survey.html
.. _API   documentation:
    http://www.equalizergraphics.com/documents/Developer/API-1.0/index.html
.. _examples: https://github.com/Eyescale/Equalizer/tree/1.2/examples
.. _Developer Documentation:
    http://www.equalizergraphics.com/doc_developer.html
.. _Documentation     Set: http://www.equalizergraphics.com/documents/Dev
    eloper/API-1.0/ch.eyescale.Equalizer.docset.zip
.. _     Developer Mailing List: http://www.equalizergraphics.com/cgi-
    bin/mailman/listinfo/eq-dev
.. _     info@equalizergraphics.com:
    mailto:info@equalizergraphics.com?subject=Equalizer%20question
.. _Eyescale: http://www.eyescale.ch
.. _info@eyescale.ch: mailto:info@eyescale.ch?subject=Equalizer%20support
