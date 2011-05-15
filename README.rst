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
scalable OpenGL applications. This release introduces the Collage library,
support for GPU compression plugins and an administrative API for runtime
configuration changes.

Equalizer 1.0 culminates over 6 years of development and decades of
experience into a feature-rich, high-performance and mature parallel
rendering framework and an object-oriented high-level network library. It is
intended for all application developers creating parallel, interactive OpenGL
applications. Equalizer 1.0 can be retrieved by downloading the `source
code`_, updating the subversion trunk to revision 5809 (``svn up -r 5809``)
or by using:
``svn co
https://equalizer.svn.sourceforge.net/svnroot/equalizer/tags/release-1.0``


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
    support cluster-based execution. Equalizer furnishes and uses the Collage
    network library, a cross-platform C++ library for building heterogenous,
    distributed applications.
-   **Support for Stereo and Immersive Environments:** Equalizer supports
    both active and passive stereo rendering, as well as head tracking and
    head-mounted displays used in immersive Virtual Reality installations.


2. New in this release
----------------------

Equalizer 1.0 contains the following features, enhancements, bug fixes and
documentation changes:


2.1. New Features
~~~~~~~~~~~~~~~~~

1.0, 1.0-beta (0.9.3): No new features

1.0-alpha (0.9.2):

-   Collage: an object-oriented network library, formerly known as
    ``eq::net``. Eventually Collage will be separated completely from
    Equalizer. (technology preview)
-   CMake build system for all supported platforms
-   Support for `GPU-CPU transfer and compression plugins`_
-   `Failure tolerance`_ during initialization
-   `Administrative API`_ for runtime configuration changes (technology
    preview)
-   `Runtime stereo switching`_ to select mono and stereo rendering at
    application runtime
-   `Slave object commit`_ supports serializing changed data on a slave
    object instance to the master instance
-   Automatic compression of distributed object data
-   Support for pixel formats with 10 bit per color component
-   `Rendering capabilities`_ allow application-dependent (de-)activation
    of channels
-   `Interruptible rendering`_ allows applications to stop rendering on
    all pending frames

0.9.1:

-   ` Subpixel compounds`_ for full-scene anti-aliasing (FSAA) or depth-
    of-field (DOF) decomposition
-   `Data distribution and data updates using reliable UDP multicast`_
-   Support for writing applications which are not using OpenGL


2.2. Enhancements
~~~~~~~~~~~~~~~~~

1.0

-   `RFE 3237701`_: Allow direct mapping of objects with known master
    node
-   Make co::Object serialization compressor configurable

1.0-beta (0.9.3):

-   `RFE 3076532`_: Configurable texture filtering for frame
-   `RFE 3175000`_: Add Window FPS event
-   `RFE 3170562`_: Support distributing segments to multiple channels
-   `RFE 3160123`_: Support for orthographic stereo frustum
-   `RFE 2990876`_: Mouse wheel support
-   Upgraded `GLEW`_ to version 1.5.8

1.0-alpha (0.9.2):

-   Support for Windows 7
-   Upgraded `GLEW`_ to version 1.5.7.3
-   Structured `error reporting`_
-   Statistics overlay: Add compression and download ratio, render
    overlay without usage of depth buffer
-   CPU compression plugins: allow different output from input token
-   New command line argument ``--eq-logfile``
-   New compound auto stereo mode detection (active, anaglyph, passive)
-   `RFE 3098130`_: Support hostnames for multicast connections
-   `RFE 2809019`_: Specify connection from a config file when using
    appNode
-   `RFE 3086646`_: Load and view equalizer: consider assemble time
-   `RFE 3036064`_: View and load equalizer should consider network times
-   `RFE 2927688`_: Loadbalancer tile sizes should not exceed channel
    PVP's

0.9.1:

-   Support for Mac OS X 10.6 Snow Leopard
-   `Tile and range boundaries`_ for the load equalizer
-   New `eq::util::Accum`_ class for accumulation operations using an FBO
    or the OpenGL accumulation buffer
-   Multiple windows on the same pipe can join the same software swap
    barrier
-   `Configurable message pump`_
-   Added attached and detach notification to ``co::Object``.


2.3. Optimizations
~~~~~~~~~~~~~~~~~~

1.0:

-   Optimize Collage command cache performance
-   Optimize short int data transmission

1.0-beta (0.9.3): No new optimizations

1.0-alpha (0.9.2):

-   Mac OS X: Use SpinLocks over pthread locks for significantly improved
    performance in various places
-   Collage: Simplify and speed up command packet dispatch and invocation
-   Collage: Optimize RSP multicast using sliding ack window with early
    acks
-   Collage: Send object instance data during registration to accelerate
    object mapping

0.9.1:

-   Configurable object serialization buffer size
-   Performance optimization for image compression
-   Reduce memory footprint for eq::net::Objects with change type DELTA


2.4. Examples
~~~~~~~~~~~~~

1.0, 1.0-beta (0.9.3): No significant example changes

1.0-alpha (0.9.2):

-   New `OSGScaleViewer`_ example, evolved from eqOSG contribution
-   EqPly: Run-time configurable image quality
-   EqPly: Run-time toggling of idle anti-aliasing

0.9.1:

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

1.0-alpha and later: API changes are tracked in `CHANGES.txt`_.

0.9.1:

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

1.0:

-   Full `API documentation`_ for the public Equalizer API.

1.0-beta (0.9.3):

-   The `Programming and User Guide`_ has been extended to 102 pages and
    56 figures.
-   Full `API documentation`_ for the public Equalizer API.

1.0-alpha (0.9.2):

-   Full `API documentation`_ for the public Equalizer API.
-   `Error handling`_ structures error reporting, mostly during
    initialization.
-   `Interruptible rendering`_ allows applications to stop rendering on
    all pending frames.
-   `Rendering capabilities`_ allow application-dependent deactivation of
    channels.
-   `Administrative API`_ for runtime configuration changes.
-   `Runtime stereo switching`_ allows to change the rendering between
    mono and stereo at runtime, with different scalability compounds.
-   `Failure tolerance`_ during initialization.

0.9.1:

-   `Subpixel Compound`_ for full-scene anti-aliasing (FSAA) or depth-of-
    field (DOF).
-   `Data distribution and data updates using reliable UDP multicast`_.


2.8. Bug Fixes
~~~~~~~~~~~~~~

Equalizer 1.0 includes various bugfixes over the 0.9 release, including the
following:

1.0:

-   `3264449`_: View/segment channel viewport failure when using pvp
-   `3234693`_: Delete 'channel' views when view is deleted using admin
    API
-   Fixed some minor memory leak found with valgrind
-   `3213628`_: Win32: co::base::Condition leaks memory on destruction
-   `3206311`_: Node process does not exit after failed Node::configInit
-   `3203934`_: Channel statistics-related race
-   `3201871`_: View equalizer: changing capabilities does not update
    nPipes
-   `3199651`_: Improve handling of unsupported connections
-   `3196124`_: View mapping may fail when Config::update is used

1.0-beta (0.9.3):

-   `3190280`_: Wrong compressor when switching image format
-   `3185777`_: occasional segfault on exit
-   `3185685`_: Assembly fails if the channel has no view
-   `3183597`_: Multicast / RSP assertions on exit
-   `3175659`_: Excessive memory usage with serialization compression
-   `3175117`_: Occasional hang on exit
-   `3172604`_: Near plane adaptation fails sometimes
-   `3171582`_: Assertion during admin mapping
-   `3168709`_: AGL/GLX: (half) float PBuffers not implemented
-   `3166620`_: send-on-register should not send to self
-   `3166560`_: Win32: RNG not random
-   `3166619`_: Win32: PipeConnection too slow
-   `3166437`_: Startup deadlock
-   `3165985`_: Send-on-register and multicast don't match
-   `3161488`_: FPS decreases over time
-   `3159980`_: Constant reallocation of decompression engines
-   `3158106`_: 'Self' compressor plugin init fails
-   `3156359`_: 'Admin' passive window uses anaglyph in
    1-pipe.stereo.anagly
-   `3156321`_: Delta object commits call getInstanceData(), not pack()
-   `3156114`_: Release build / NDEBUG issues
-   `3156103`_: Add default appNode connection for multi-node configs
-   `3156102`_: 32/64 bit interoperability broken
-   `3156100`_: MSVC / gcc interoperability broken
-   `3155603`_: XCode-build binaries don't find server library
-   `3155543`_: Missing fragments when using YUV GPU Compressor
-   `3155530`_: Assertion server/config.cpp:875
-   `3155511`_: Wrong detection of AUTO stereo mode
-   `3155397`_: GL_INVALID_OPERATION when switching layouts
-   `3155386`_: Admin copies are never synced
-   `3138516`_: eVolve is broken
-   `2985875`_: View user data mapping fails during initialization
-   `2934387`_: Ubuntu: GLX problem with PBuffer
-   `2003195`_: Ortho frustra ignores eye offset

1.0-alpha (0.9.2):

-   `3152421`_: Distinguish window and channel pointer events
-   `2976899`_: Config::finishFrame deadlocks when no nodes are active
-   `2994111`_: Rounding errors with 2D LB and 16 sources
-   `3137933`_: GLXEW init buggy
-   `2882248`_: Spurious network deadlocks on Win32
-   `3071764`_: GLX: No distinction between lowercase and uppercase keys

0.9.1:

-   `2873353`_: Usage of ext/hash_map and -Werror causes compiler error
-   `2834063`_: eqPly and eVolve freezes on Mac with glX
-   `2828269`_: eVolve depth compositing is broken
-   `2642034`_: Win32: max 64 connections possible
-   `2874188`_: Occasional lockup at shutdown


2.9. Known Bugs
~~~~~~~~~~~~~~~

The following bugs were known at release time. Please file a `Bug Report`_ if
you find any other issue with this release.

-   `2796444`_: Race during simultaneous node connect
-   `2609161`_: zoom: depth readback does not work
-   `2556940`_: zoom: FBO usage
-   `1854929`_: eqPly GLSL shader has artefacts


3. About
--------

Equalizer is a cross-platform toolkit, designed to run on any modern
operating system, including all Unix variants and the Windows operating
system. A `compatibility matrix`_ can be found on the Equalizer website.

Equalizer requires at least `OpenGL 1.1`_, but uses newer OpenGL features
when available. Version 1.0 has been tested on:


3.1. Operating System Support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Equalizer uses CMake to create a platform-specific build environment. The
following platforms and build environments are tested:

-   **Linux:** Ubuntu 10.04, 10.10 (Makefile, i386, x64)
-   **Windows:** XP and 7 (Visual Studio 2008, i386, x64)
-   **Mac OS X:** 10.5, 10.6 (Makefile, XCode, i386, x64)


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

0.9.1:

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
    http://www.equalizergraphics.com/downloads/Equalizer-1.0.tar.gz
.. _detailed feature list: /features.html
.. _GPU-CPU       transfer and compression plugins: http://www.equalizerg
    raphics.com/documents/Developer/API/plugins_2compressor_8h.html#_details
.. _Failure      tolerance:
    http://www.equalizergraphics.com/documents/design/nodeFailure.html
.. _Administrative       API:
    http://www.equalizergraphics.com/documents/design/admin.html
.. _Runtime       stereo switching:
    http://www.equalizergraphics.com/documents/design/stereoSwitch.html
.. _Slave       object commit:
    http://www.equalizergraphics.com/documents/design/admin.html#slaveWrite
.. _Rendering capabilities:
    http://www.equalizergraphics.com/documents/design/Capabilities.html
.. _Interruptible       rendering:
    http://www.equalizergraphics.com/documents/design/stopFrames.html
.. _       Subpixel compounds:
    http://www.equalizergraphics.com/documents/design/subpixelCompound.html
.. _Data distribution and data updates using reliable UDP multicast:
    http://www.equalizergraphics.com/documents/design/multicast.html
.. _RFE   3237701: https://sourceforge.net/tracker/index.php?func=detail&
    aid=3237701&group_id=170962&atid=856212
.. _RFE 3076532: https://sourceforge.net/tracker/index.php?func=detail&ai
    d=3076532&group_id=170962&atid=856212
.. _RFE 3175000: https://sourceforge.net/tracker/index.php?func=detail&ai
    d=3175000&group_id=170962&atid=856212
.. _RFE 3170562: https://sourceforge.net/tracker/index.php?func=detail&ai
    d=3170562&group_id=170962&atid=856212
.. _RFE 3160123: https://sourceforge.net/tracker/index.php?func=detail&ai
    d=3160123&group_id=170962&atid=856212
.. _RFE 2990876: https://sourceforge.net/tracker/index.php?func=detail&ai
    d=2990876&group_id=170962&atid=856212
.. _GLEW: http://glew.sourceforge.net
.. _GLEW: http://glew.sourceforge.net
.. _error reporting:
    http://www.equalizergraphics.com/documents/design/errorHandling.html
.. _RFE 3098130: https://sourceforge.net/tracker/index.php?func=detail&ai
    d=3098130&group_id=170962&atid=856212
.. _RFE   2809019: https://sourceforge.net/tracker/?func=detail&aid=28090
    19&group_id=170962&atid=856212
.. _RFE       3086646: https://sourceforge.net/tracker/?func=detail&aid=3
    086646&group_id=170962&atid=856212
.. _RFE       3036064: https://sourceforge.net/tracker/?func=detail&aid=3
    036064&group_id=170962&atid=856212
.. _RFE       2927688: https://sourceforge.net/tracker/?func=detail&aid=2
    927688&group_id=170962&atid=856212
.. _Tile and range boundaries: http://www.equalizergraphics.com/documents
    /design/loadBalancing.html#boundaries
.. _eq::util::Accum: http://www.equalizergraphics.com/documents/Developer
    /API/classeq_1_1util_1_1Accum.html
.. _Configurable   message pump: https://sourceforge.net/tracker/?func=de
    tail&aid=2902505&group_id=170962&atid=856212
.. _OSGScaleViewer: http://www.equalizergraphics.com/documents/WhitePaper
    s/OpenSceneGraphClustering.pdf
.. _CHANGES.txt: https://equalizer.svn.sourceforge.net/svnroot/equalizer/
    tags/release-1.0/CHANGES.txt
.. _API       documentation:
    http://www.equalizergraphics.com/documents/Developer/API-1.0/index.html
.. _Programming and       User Guide:
    http://www.equalizergraphics.com/survey.html
.. _API       documentation:
    http://www.equalizergraphics.com/documents/Developer/API-0.9.3/index.html
.. _API       documentation:
    http://www.equalizergraphics.com/documents/Developer/API-0.9.2/index.html
.. _Error       handling:
    http://www.equalizergraphics.com/documents/design/errorHandling.html
.. _Interruptible       rendering:
    http://www.equalizergraphics.com/documents/design/stopFrames.html
.. _Rendering       capabilities:
    http://www.equalizergraphics.com/documents/design/Capabilities.html
.. _Administrative       API:
    http://www.equalizergraphics.com/documents/design/admin.html
.. _Runtime       stereo switching:
    http://www.equalizergraphics.com/documents/design/stereoSwitch.html
.. _Failure       tolerance:
    http://www.equalizergraphics.com/documents/design/nodeFailure.html
.. _Subpixel Compound: /documents/design/subpixelCompound.html
.. _Data distribution and data       updates using reliable UDP
    multicast: /documents/design/multicast.html
.. _3264449: https://sourceforge.net/tracker/?func=detail&aid=3264449&gro
    up_id=170962&atid=856209
.. _3234693: https://sourceforge.net/tracker/?func=detail&aid=3234693&gro
    up_id=170962&atid=856209
.. _3213628: https://sourceforge.net/tracker/?func=detail&aid=3213628&gro
    up_id=170962&atid=856209
.. _3206311: https://sourceforge.net/tracker/?func=detail&aid=3206311&gro
    up_id=170962&atid=856209
.. _3203934: https://sourceforge.net/tracker/?func=detail&aid=3203934&gro
    up_id=170962&atid=856209
.. _3201871: https://sourceforge.net/tracker/?func=detail&aid=3201871&gro
    up_id=170962&atid=856209
.. _3199651: https://sourceforge.net/tracker/?func=detail&aid=3199651&gro
    up_id=170962&atid=856209
.. _3196124: https://sourceforge.net/tracker/?func=detail&aid=3196124&gro
    up_id=170962&atid=856209
.. _3190280: https://sourceforge.net/tracker/?func=detail&aid=3190280&gro
    up_id=170962&atid=856209
.. _3185777: https://sourceforge.net/tracker/?func=detail&aid=3185777&gro
    up_id=170962&atid=856209
.. _3185685: https://sourceforge.net/tracker/?func=detail&aid=3185685&gro
    up_id=170962&atid=856209
.. _3183597: https://sourceforge.net/tracker/?func=detail&aid=3183597&gro
    up_id=170962&atid=856209
.. _3175659: https://sourceforge.net/tracker/?func=detail&aid=3175659&gro
    up_id=170962&atid=856209
.. _3175117: https://sourceforge.net/tracker/?func=detail&aid=3175117&gro
    up_id=170962&atid=856209
.. _3172604: https://sourceforge.net/tracker/?func=detail&aid=3172604&gro
    up_id=170962&atid=856209
.. _3171582: https://sourceforge.net/tracker/?func=detail&aid=3171582&gro
    up_id=170962&atid=856209
.. _3168709: https://sourceforge.net/tracker/?func=detail&aid=3168709&gro
    up_id=170962&atid=856209
.. _3166620: https://sourceforge.net/tracker/?func=detail&aid=3166620&gro
    up_id=170962&atid=856209
.. _3166560: https://sourceforge.net/tracker/?func=detail&aid=3166560&gro
    up_id=170962&atid=856209
.. _3166619: https://sourceforge.net/tracker/?func=detail&aid=3166619&gro
    up_id=170962&atid=856209
.. _3166437: https://sourceforge.net/tracker/?func=detail&aid=3166437&gro
    up_id=170962&atid=856209
.. _3165985: https://sourceforge.net/tracker/?func=detail&aid=3165985&gro
    up_id=170962&atid=856209
.. _3161488: https://sourceforge.net/tracker/?func=detail&aid=3161488&gro
    up_id=170962&atid=856209
.. _3159980: https://sourceforge.net/tracker/?func=detail&aid=3159980&gro
    up_id=170962&atid=856209
.. _3158106: https://sourceforge.net/tracker/?func=detail&aid=3158106&gro
    up_id=170962&atid=856209
.. _3156359: https://sourceforge.net/tracker/?func=detail&aid=3156359&gro
    up_id=170962&atid=856209
.. _3156321: https://sourceforge.net/tracker/?func=detail&aid=3156321&gro
    up_id=170962&atid=856209
.. _3156114: https://sourceforge.net/tracker/?func=detail&aid=3156114&gro
    up_id=170962&atid=856209
.. _3156103: https://sourceforge.net/tracker/?func=detail&aid=3156103&gro
    up_id=170962&atid=856209
.. _3156102: https://sourceforge.net/tracker/?func=detail&aid=3156102&gro
    up_id=170962&atid=856209
.. _3156100: https://sourceforge.net/tracker/?func=detail&aid=3156100&gro
    up_id=170962&atid=856209
.. _3155603: https://sourceforge.net/tracker/?func=detail&aid=3155603&gro
    up_id=170962&atid=856209
.. _3155543: https://sourceforge.net/tracker/?func=detail&aid=3155543&gro
    up_id=170962&atid=856209
.. _3155530: https://sourceforge.net/tracker/?func=detail&aid=3155530&gro
    up_id=170962&atid=856209
.. _3155511: https://sourceforge.net/tracker/?func=detail&aid=3155511&gro
    up_id=170962&atid=856209
.. _3155397: https://sourceforge.net/tracker/?func=detail&aid=3155397&gro
    up_id=170962&atid=856209
.. _3155386: https://sourceforge.net/tracker/?func=detail&aid=3155386&gro
    up_id=170962&atid=856209
.. _3138516: https://sourceforge.net/tracker/?func=detail&aid=3138516&gro
    up_id=170962&atid=856209
.. _2985875: https://sourceforge.net/tracker/?func=detail&aid=2985875&gro
    up_id=170962&atid=856209
.. _2934387: https://sourceforge.net/tracker/?func=detail&aid=2934387&gro
    up_id=170962&atid=856209
.. _2003195: https://sourceforge.net/tracker/?func=detail&aid=2003195&gro
    up_id=170962&atid=856209
.. _3152421: https://sourceforge.net/tracker/?func=detail&aid=3152421&gro
    up_id=170962&atid=856209
.. _2976899: https://sourceforge.net/tracker/?func=detail&aid=2976899&gro
    up_id=170962&atid=856209
.. _2994111: https://sourceforge.net/tracker/?func=detail&aid=2994111&gro
    up_id=170962&atid=856209
.. _3137933: https://sourceforge.net/tracker/?func=detail&aid=3137933&gro
    up_id=170962&atid=856209
.. _2882248: https://sourceforge.net/tracker/?func=detail&aid=2882248&gro
    up_id=170962&atid=856209
.. _3071764: https://sourceforge.net/tracker/?func=detail&aid=3071764&gro
    up_id=170962&atid=856209
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
.. _2796444: http://sourceforge.net/tracker/index.php?func=detail&aid=279
    6444&group_id=170962&atid=856209
.. _2609161: http://sourceforge.net/tracker/index.php?func=detail&aid=260
    9161&group_id=170962&atid=856209
.. _2556940: http://sourceforge.net/tracker/index.php?func=detail&aid=255
    6940&group_id=170962&atid=856209
.. _1854929: http://sourceforge.net/tracker/index.php?func=detail&aid=185
    4929&group_id=170962&atid=856209
.. _compatibility matrix:
    http://www.equalizergraphics.com/compatibility.html
.. _OpenGL 1.1: http://www.opengl.org
.. _hard-copy: http://www.lulu.com/product/paperback/equalizer-10
    -programming-and-user-guide/15165632
.. _online: http://www.equalizergraphics.com/survey.html
.. _API   documentation:
    http://www.equalizergraphics.com/documents/Developer/API-1.0/index.html
.. _examples: http://www.equalizergraphics.com/cgi-
    bin/viewvc.cgi/tags/release-1.0/examples/
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
.. _Patch: http://equalizer.svn.sourceforge.net/viewvc/equalizer/tags/rel
    ease-0.9.1/patches/wgl_no_glew.patch?view=markup
