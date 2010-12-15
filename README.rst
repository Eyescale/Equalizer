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
scalable OpenGL applications. Equalizer 0.9.1 introduces reliable multicast
data distribution and subpixel compounds for full-scene anti-aliasing and
depth-of-field decomposition.

Equalizer 0.9.1 is a developer release, representing a stable snapshot of the
development tree after the 0.9 release. Equalizer 0.9.1 can be retrieved by
downloading the `source code`_. , updating the subversion trunk to revision
4082 (``svn up -r 4082``) or by using:
``svn co
https://equalizer.svn.sourceforge.net/svnroot/equalizer/tags/release-0.9.1``


1.1. Features
~~~~~~~~~~~~~

Equalizer provides the following major features to facilitate the development
and deployment of scalable OpenGL applications. A `detailed feature list`_
can be found on the Equalizer website.

-   **Runtime Configurability:** An Equalizer application is configured
    at runtime and can be deployed on laptops, multi-GPU workstations and
    large-scale visualization clusters without recompilation.
-   **Runtime Scalability:** An Equalizer application can benefit from
    multiple graphics cards, processors and computers to scale rendering
    performance, visual quality and display size.
-   **Distributed Execution:** Equalizer applications can be written to
    support cluster-based execution. The task of distributing the application
    data is facilitated by support for versioned, distributed objects.
-   **Support for Stereo and Immersive Environments:** Equalizer supports
    both active and passive stereo rendering, as well as head tracking and
    head-mounted displays used in immersive Virtual Reality installations.


2. New in this release
----------------------

Equalizer 0.9.1 contains the following features, enhancements, bug fixes and
documentation changes:


2.1. New Features
~~~~~~~~~~~~~~~~~

-   ` Subpixel compounds`_ for full-scene anti-aliasing (FSAA) or depth-
    of-field (DOF) decomposition
-   `Data distribution and data updates using reliable UDP multicast`_
-   Support for writing applications which are not using OpenGL


2.2. Enhancements
~~~~~~~~~~~~~~~~~

-   Support for Mac OS X 10.6 Snow Leopard
-   `Tile and range boundaries`_ for the load equalizer
-   New `eq::util::Accum`_ class for accumulation operations using an FBO
    or the OpenGL accumulation buffer
-   Multiple windows on the same pipe can join the same software swap
    barrier
-   `Configurable message pump`_


2.3. Optimizations
~~~~~~~~~~~~~~~~~~

-   Configurable object serialization buffer size
-   Performance optimization for image compression
-   Reduce memory footprint for eq::net::Objects with change type DELTA


2.4. Examples
~~~~~~~~~~~~~

-   EqPly: added anti-aliasing when the application is idle
-   EqPly: recursively search directories for models
-   EqPly: switch to faster VBO rendering on OSX


2.5. Tools
~~~~~~~~~~

-   No Changes


2.6. API Changes
~~~~~~~~~~~~~~~~

The following changes breaking compatibility with Equalizer 0.6 source code
were made:

-   The utility classes ``Accum``, ``AccumBufferObject``,
    ``FrameBufferObject`` and ``Texture`` where moved from the ``eq`` to the
    ``eq::util`` namespace.
-   ``eq::Window::getColorType`` has been changed to ``getColorFormat``
    for consistency.
-   The font handling provided by ``eq::Window`` has been refactored for
    non-OpenGL rendering support.


2.7. Documentation
~~~~~~~~~~~~~~~~~~

The following documentation has been added or substantially improved since
the last release:

-   The `Programming and User Guide`_ has been extended to 91 pages and
    53 figures.
-   The Accumulation Buffer Object API has been added to the Programming
    Guide.
-   `The Subpixel compound`_ provides FSAA and DOF decomposition
-   `Multicast Data Transfer`_


2.8. Bug Fixes
~~~~~~~~~~~~~~

Equalizer 0.9.1 includes various bugfixes over the 0.9 release, including the
following:

-   `2873353`_: Usage of ext/hash_map and -Werror causes compiler error
-   `2834063`_: eqPly and eVolve freezes on Mac with glX
-   `2828269`_: eVolve depth compositing is broken
-   `2642034`_: Win32: max 64 connections possible
-   `2874188`_: Lockup at shutdown


2.9. Known Bugs
~~~~~~~~~~~~~~~

The following bugs were known at release time. Please file a `Bug Report`_ if
you find any other issue with this release.

-   `2934387`_: Ubuntu: GLX problem with PBuffer
-   `2931913`_ eqPly: DFR + 2D + Idle AA does not work
-   `2843849`_: 64-bit bug in eVolveConverter
-   `2796444`_: Race during simultaneous node connect
-   `2609161`_: zoom: depth readback does not work
-   `2556940`_: zoom: FBO usage
-   `2003195`_: Ortho frustra ignores eye offset
-   `1854929`_: eqPly GLSL shader has artefacts


3. About
--------

Equalizer is a cross-platform toolkit, designed to run on any modern
operating system, including all Unix variants and the Windows operating
system. A `compatibility matrix`_ can be found on the Equalizer website.

Equalizer requires at least `OpenGL 1.1`_, but uses newer OpenGL features
when available. Version 0.9.1 has been tested on:


3.1. Operating System Support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-   **Linux:** Ubuntu 9.04, 9.10 (i386, x64)
-   **Windows:** XP with Visual Studio 2005 (i386, x64)
-   **Mac OS X:** 10.5, 10.6 (i386)


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

-   `Patch`_ to fix occasional compilation errors on Windows when using
    EQ_IGNORE_GLEW.

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
    http://www.equalizergraphics.com/downloads/Equalizer-0.9.1.tar.gz
.. _detailed feature list: /features.html
.. _       Subpixel compounds:
    http://www.equalizergraphics.com/documents/design/subpixelCompound.html
.. _Data distribution and data updates using reliable UDP multicast:
    http://www.equalizergraphics.com/documents/design/multicast.html
.. _Tile and range boundaries: http://www.equalizergraphics.com/documents
    /design/loadBalancing.html#boundaries
.. _eq::util::Accum: http://www.equalizergraphics.com/documents/Developer
    /API/classeq_1_1util_1_1Accum.html
.. _Configurable message pump: https://sourceforge.net/tracker/?func=deta
    il&aid=2902505&group_id=170962&atid=856212
.. _Programming and       User Guide:
    http://www.equalizergraphics.com/survey.html
.. _The   Subpixel compound:
    http://www.equalizergraphics.com/documents/design/subpixelCompound.html
.. _Multicast Data Transfer:
    http://www.equalizergraphics.com/documents/design/multicast.html
.. _2873353: https://sourceforge.net/tracker/?func=detail&aid=2873353&gro
    up_id=170962&atid=856209
.. _2834063: https://sourceforge.net/tracker/?func=detail&aid=2834063&gro
    up_id=170962&atid=856209
.. _2828269: https://sourceforge.net/tracker/?func=detail&aid=2828296&gro
    up_id=170962&atid=856209
.. _2642034: http://sourceforge.net/tracker/index.php?func=detail&aid=264
    2034&group_id=170962&atid=856209
.. _2874188: https://sourceforge.net/tracker/?func=detail&aid=2874188&gro
    up_id=170962&atid=856209
.. _Bug   Report:
    http://sourceforge.net/tracker/?atid=856209&group_id=170962&func=browse
.. _2934387: https://sourceforge.net/tracker/?func=detail&aid=2934387grou
    p_id=170962&atid=856209
.. _2931913: https://sourceforge.net/tracker/?func=detail&aid=2931913&gro
    up_id=170962&atid=856209
.. _2843849: https://sourceforge.net/tracker/?func=detail&aid=2843849&gro
    up_id=170962&atid=856209
.. _2796444: http://sourceforge.net/tracker/index.php?func=detail&aid=279
    6444&group_id=170962&atid=856209
.. _2609161: http://sourceforge.net/tracker/index.php?func=detail&aid=260
    9161&group_id=170962&atid=856209
.. _2556940: http://sourceforge.net/tracker/index.php?func=detail&aid=255
    6940&group_id=170962&atid=856209
.. _2003195: http://sourceforge.net/tracker/index.php?func=detail&aid=200
    3195&group_id=170962&atid=856209
.. _1854929: http://sourceforge.net/tracker/index.php?func=detail&aid=185
    4929&group_id=170962&atid=856209
.. _compatibility matrix:
    http://www.equalizergraphics.com/compatibility.html
.. _OpenGL 1.1: http://www.opengl.org
.. _hard-copy: http://www.lulu.com/content/paperback-book/equalizer-09
    -programming-and-user-guide/7501548
.. _online: http://www.equalizergraphics.com/survey.html
.. _API documentation:
    http://www.equalizergraphics.com/documents/Developer/API/index.html
.. _examples: http://www.equalizergraphics.com/cgi-
    bin/viewvc.cgi/tags/release-0.9.1/examples/
.. _Developer Documentation:
    http://www.equalizergraphics.com/doc_developer.html
.. _Documentation     Set: http://www.equalizergraphics.com/documents/Dev
    eloper/API/ch.eyescale.Equalizer.docset.zip
.. _     Developer Mailing List: http://www.equalizergraphics.com/cgi-
    bin/mailman/listinfo/eq-dev
.. _     info@equalizergraphics.com:
    mailto:info@equalizergraphics.com?subject=Equalizer%20question
.. _Eyescale: http://www.eyescale.ch
.. _info@eyescale.ch: mailto:info@eyescale.ch?subject=Equalizer%20support
.. _Patch: http://equalizer.svn.sourceforge.net/viewvc/equalizer/tags/rel
    ease-0.9.1/patches/wgl_no_glew.patch?view=markup
