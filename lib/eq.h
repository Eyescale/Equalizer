
/* Copyright (c) 2005-2008, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#ifndef EQ_H
#define EQ_H

#include <eq/base/defines.h>

#include <eq/client/channelStatistics.h>
#include <eq/client/client.h>
#include <eq/client/compositor.h>
#include <eq/client/config.h>
#include <eq/client/configEvent.h>
#include <eq/client/configParams.h>
#include <eq/client/event.h>
#include <eq/client/frame.h>
#include <eq/client/frameData.h>
#include <eq/client/global.h>
#include <eq/client/image.h>
#include <eq/client/init.h>
#include <eq/client/log.h>
#include <eq/client/matrix4.h>
#include <eq/client/node.h>
#include <eq/client/nodeFactory.h>
#include <eq/client/objectManager.h>
#include <eq/client/packets.h>
#include <eq/client/pipe.h>
#include <eq/client/server.h>
#include <eq/client/version.h>
#include <eq/client/view.h>
#include <eq/client/windowSystem.h>

#ifdef AGL
#  include <eq/client/aglWindow.h>
#  include <eq/client/aglEventHandler.h>
#endif
#ifdef GLX
#  include <eq/client/glXWindow.h>
#  include <eq/client/glXEventHandler.h>
#endif
#ifdef WGL
#  include <eq/client/wglWindow.h>
#  include <eq/client/wglEventHandler.h>
#endif

#include <eq/base/sleep.h>
#include <eq/net/net.h>
#include <eq/util/util.h>

#include <vmmlib/vmmlib.h>

#ifdef EQ_USE_DEPRECATED
namespace eqBase = ::eq::base;
namespace eqNet  = ::eq::net;
#endif

// Copy&paste RelNotes.html content here

/*! \mainpage Equalizer Documentation

<a name="introduction"></a>
<h2>1. Introduction</h2>
<p>
  Welcome to Equalizer, a framework for the development and deployment of
  parallel, scalable OpenGL applications. Equalizer 0.6 delivers major new
  features, most notably support for DPlex compounds and automatic
  load-balancing.
</p>
<p>
  Equalizer 0.6 can be retrieved by downloading the
  <a href="http://www.equalizergraphics.com/downloads/Equalizer-0.6.tar.gz">source
    code</a>.
</p>


<a name="features"></a>
<h3>1.1. Features</h3>
<p>
  Equalizer provides the following major features to facilitate the development
  and deployment of scalable OpenGL applications. A
  <a href="/features.html">detailed feature list</a> can be found on the
  Equalizer website.
</p>
<ul>
  <li><b>Runtime Configurability:</b> An Equalizer application can run on any
    configuration, from laptops to large-scale visualization clusters, without
    recompilation. The runtime configuration is externalized from the
    application to a system-wide resource server.</li>
  <li><b>Runtime Scalability:</b> An Equalizer application can aggregate
    multiple CPU's, GPU's and computers to increase the rendering performance of
    a single or multiple views.</li>
  <li><b>Distributed Execution:</b> Equalizer applications can be written to
    support cluster-based execution. The task of distributing the application
    data is facilitated by support for versioned, distributed objects.</li>
  <li><b>Support for Stereo and Immersive Environments:</b> Equalizer supports
    both active and passive stereo rendering, as well as head tracking, used in
    immersive Virtual Reality installations.</li>
</ul>

<a name="new"></a>
<h2>2. New in this release</h2>
<p>
  Equalizer 0.6 contains the following features, enhancements, bug fixes and
  documentation changes:
</p>

<a name="newFeatures"></a>
<h3>2.1. New Features</h3>
<ul>
  <li>Automatic <a href="/scalability.html#lb">2D and DB
      load-balancing</a></li>
  <li>Support for <a href="/scalability.html#DPlex">DPlex</a> (time-multiplex,
    SFR) compounds</li>
  <li><a href="/documents/design/statisticsOverlay.html">Statistics Overlay</a> 
    to understand and eliminate bottlenecks in the rendering pipeline</li>
  <li>Easy configuration of 
    <a href="/documents/design/threads.html#sync">thread synchronization
      model</a></li>
  <li><a href="http://www.equalizergraphics.com/documents/design/view.html">View
      API</a> to modify the view parameters during rendering</li>
  <li>New OSWindow interface to simplify window system integration</li>
  <li>New 2D bitmap font to draw text</li>
  <li>Support for orthographic projections</li>
  <li>Support for OpenGL accumulation buffer and multisample setup</li>
  <li>Support for using <a href="http://paracomp.sourceforge.net/">Paracomp</a>
    as a
    <a href="/cgi-bin/viewvc.cgi/trunk/src/README.paracomp?view=markup">compositing
      backend</a></li>
</ul>

<a name="enhancements"></a>
<h3>2.2. Enhancements</h3>
<ul>
  <li>Upgraded <a href="http://glew.sourceforge.net">GLEW</a> to version
    1.5.1</li>
  <li>Automatic image compression for 'slow' (< 2GBit) connections</li>
  <li>Alpha-blending support in the CPU-based compositor</li>
  <li>Support for two-dimensional pixel compound kernels</li>
  <li>Support for using multiple clients with the netperf benchmark tool</li>
  <li>Support window swap buffer vertical retrace synchronization on WGL</li>
  <li>Add show and hide window events</li>
</ul>

<a name="optimizations"></a>
<h3>2.3. Optimizations</h3>
<ul>
  <li>Improved overall performance by using atomic operations for reference
    counted objects</li>
  <li>Improved performance when using non-threaded pipes</li>
  <li>Asynchronous network frame transmission during compositing</li>
  <li>Assertions are disabled in release builds</li>
  <li>Switch to CriticalSection for Win32 locks</li>
</ul>

<a name="examples"></a>
<h3>2.4. Examples</h3>
<ul>
  <li>eqPly: network-based instead of file-based model distribution</li>
  <li>eqPly: added head-tracking emulation</li>
  <li>eqPly: implemented event-driven execution</li>
</ul>

<a name="changes"></a>
<h3>2.5. API Changes</h3>
<ul>
  <li>The compound attribute <code>UPDATE_FOV</code> has been removed, since
    view updates are handled by the application, using the new View API.</li>
  <li><code>Event::RESIZE</code> has been deprecated. Use
    <code>Event::WINDOW_RESIZE</code> instead.</li>
  <li>The new <code>OSWindow</code> interface moved all window system
    functionality from the <code>eq::Window</code> to different subclasses
    of <code>OSWindow</code>. Applications integrating with their own windowing
    code have to implement an <code>OSWindow</code> containing all the window
    system code, and instantiate this <code>OSWindow</code>
    in <code>Window::configInitOSWindow</code>. Please refer to the Programming
    Guide for a detailed description of the Window System Interface.</li>
  <li>The <code>OSWindow</code> interface also caused some cleanups in the event
    handling. Most notably, the classes <code>ChannelEvent</code>
    and <code>WindowEvent</code> are now unneeded and have been removed. The
    former base class <code>Event</code> is now used in the appropriate
    places.</li>
  <li>The <code>eqBase</code>, <code>eqNet</code> and <code>eqServer</code>
    namespaces have been renamed to <code>eq::base</code>, <code>eq::net</code>
    and <code>eq::server</code>, respectively. Application developers are
    encouraged to make the necessary changes, but can
    define <code>EQ_USE_DEPRECATED</code> if these changes are not
    feasible.</li>
</ul>

<a name="documentation"></a>
<h3>2.6. Documentation</h3>
<ul>
  <li>The <a href="/survey.html">Programming Guide</a> has been extended to 62
    pages and 37 figures.</li>
  <li><a href="/documents/design/view.html">View API</a></li>
  <li><a href="/documents/design/statisticsOverlay.html">Statistics
      Overlay</a></li>
</ul>

<a name="bugfixes"></a>
<h3>2.7. Bug Fixes</h3>
<p>
  Equalizer 0.6 includes various bugfixes over the 0.5 release, including
  the following:
</p>
<ul>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2369549&group_id=170962&atid=856209">2369549</a>:
  Crash in wglSwapIntervalEXT</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2284482&group_id=170962&atid=856209">2284482</a>:
  Head tracking broken with VS release builds</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2263716&group_id=170962&atid=856209">2263716</a>:
    5-channel.cave.eqc has wrong top/bottom wall</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2166278&group_id=170962&atid=856209">2166278</a>:
    eq::net::Object with instance never uses pack/unpack</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2149563&group_id=170962&atid=856209">2149563</a>:
    WGL: Update region is reset by event handler</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2047816&group_id=170962&atid=856209">2047816</a>:
    name field in config missing</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2033860&group_id=170962&atid=856209">2033860</a>:
    frameDrawFinish called too early and too often</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2032643&group_id=170962&atid=856209">2032643</a>:
    eqHello: process-local server object gets not deleted.</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2032631&group_id=170962&atid=856209">2032631</a>:
    Nullpointer exception and crash</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2032643&group_id=170962&atid=856209">2032643</a>:
    process-local server object gets not deleted</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2033860&group_id=170962&atid=856209">2033860</a>:
    frameDrawFinish called too early and too often</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2149563&group_id=170962&atid=856209">2149563</a>:
    WGL: Update region is reset by event handler</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2026837&group_id=170962&atid=856209">2026837</a>:
    swapbarrier calls increase with single-buffered windows</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1997751&group_id=170962&atid=856209">1997751</a>:
    defines for EQ_BIT17 - EQ_BIT20 are wrong</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1996988&group_id=170962&atid=856209">1996988</a>:
    AGL reports key press twice</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1997579&group_id=170962&atid=856209">1997579</a>:
    Important WGL key events not reported</li>
  <li>Fixed a big memory leak in the packet handling code, as well as numerous
    small bug fixes and code cleanups</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1994798&group_id=170962&atid=856209">1994798</a>:
    Compound::getNode null pointer read</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1981854&group_id=170962&atid=856209">1981854</a>:
    AGL: PBuffer with Fullscreen broken</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1960225&group_id=170962&atid=856209">1960225</a>:
    Problems with windows getting focus on mac</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1960225&group_id=170962&atid=856209">1960098</a>:
    eqPly crashes during rendering</li>
</ul>

<a name="knownbugs"></a>
<h3>2.8. Known Bugs</h3>
<ul>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2268547&group_id=170962&atid=856209">2268547</a>:
    fullscreen windows not always active on Win32</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2151376&group_id=170962&atid=856209">2151376</a>:
    Irregular Pixel kernels do not work</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2003195&group_id=170962&atid=856209">2003195</a>:
    Ortho frusta ignore eye offset</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=2003132&group_id=170962&atid=856209">2003132</a>:
    3-wnd.DB.ds is broken on some ppc machines for eqPly</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1997583&group_id=170962&atid=856209">1997583</a>:
    eqPly: ortho frustum culling broken</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1959418&group_id=170962&atid=856209">1959418</a>:
    DB Compositing fails on MacBook with GMA X3100</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1854948&group_id=170962&atid=856209">1854948</a>:
    eVolve: lighting ignores head transformation</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1854929&group_id=170962&atid=856209">1854929</a>:
    eqPly GLSL shader has artefacts</li>
  <li><a href="http://sourceforge.net/tracker/index.php?func=detail&aid=1816670&group_id=170962&atid=856209">1816670</a>:
    eVolve: binary swap configs don't work</li>
</ul>


<a name="about"></a>
<h2>3. About</h2>
<p>
  Equalizer is a cross-platform toolkit, designed to run on any modern operating
  system, including all Unix variants and the Windows operating system. A 
  <a href="/compatibility.html">compatibility matrix</a> can be found on the
  Equalizer website.
</p>
<p>
  Equalizer requires at least <a href="http://www.opengl.org">OpenGL 1.1</a>,
  but uses newer OpenGL features when available. Version 0.5.0 has been tested
  on:
</p>

<a name="os"></a>
<h3>3.1. Operating System Support</h3>
<ul>
  <li><b>Linux:</b> Ubuntu 6.10 (x64, i386), RHEL4 (x64, i386)</li>
  <li><b>Windows:</b> XP with Visual Studio 2005 (i386, x64) and
    Cygwin (i386)</li>
  <li><b>Mac OS X:</b> 10.5 (PowerPC, i386)</li>
</ul>

<a name="ws"></a>
<h3>3.2. Window System Support</h3>
<ul>
  <li><b>X11:</b> Full support for all documented features.</li>
  <li><b>WGL:</b> Full support for all documented features.</li>
  <li><b>AGL:</b> Full support for all documented features.</li>
</ul>

<a name="documentation"></a>
<h3>3.3. Documentation</h3>
<p>
  The Programming Guide is available as a 
  <a href="http://www.lulu.com/browse/book_view.php?fCID=5006510">hard-copy</a>
  and <a href="/survey.html">online</a>. <a href="http://www.equalizergraphics.com/documents/Developer/doxies/index.html">API documentation</a> can be
  found on the Equalizer website. Equalizer does not yet have an Users Guide.
</p>
<p>
  As with any open source project, the available source code, in particular the
  shipped
  <a href="/cgi-bin/viewvc.cgi/tags/release-0.6/examples/">examples</a>
  provide a reference for developing or porting applications. The
  <a href="/doc_developer.html">Developer Documentation</a> on the website
  provides further design documents for specific features.
</p>

<a name="support"></a>
<h3>3.4. Support</h3>
<p>
  Technical questions can be posted to the 
  <a href="/cgi-bin/mailman/listinfo/eq-dev">
    Developer Mailing List</a>, or directly to
  <a href="mailto:info@equalizergraphics.com?subject=Equalizer%20question">
    info@equalizergraphics.com</a>.
</p>
<p>
  Commercial support, custom software development and porting services are
  available from <a href="http://www.eyescale.ch">Eyescale</a>. Please contact
  <a href="mailto:info@eyescale.ch?subject=Equalizer%20support">info@eyescale.ch</a>
  for further information.
</p>

 */
#endif // EQ_H
