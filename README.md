# GPU Service Discovery

[TOC]

GPU-SD is a library and daemon for the discovery and announcement of
graphics processing units using ZeroConf. It enables auto-configuration
of ad-hoc GPU clusters and multi-GPU machines.

The source code is hosted on
[github](https://github.com/Eyescale/gpu-sd) and documented [here](http://www.equalizergraphics.com/gpu-sd/API/).

## Modules

The GPU-SD library uses modules which implement discovery using
different protocols. Each module is a separate library, which can be
selectively linked by applications, limiting dependencies. Currently
available are:

- DNS_SD: Remote ZeroConf (Bonjour) discovery for GPUs announced by the daemon
- CGL: Local discovery of Carbon displays (Mac OS X only)
- GLX: Local discovery of X11 servers and screens
- WGL: Local discovery of WGL_NV_gpu_affinity, WGL_AMD_gpu_association
  or Windows displays (Windows only)

## Daemon

The daemon uses all available local modules to query local GPUs and
announces them using ZeroConf on the local network. The service type
name is "_gpu-sd". The dns_sd discovery module gathers the information
announced by all daemons on the local network. The following protocol is
used by the daemon:

* Session=default|&lt;string&gt;
* GPU Count=&lt;integer&gt;
* GPU&lt;integer&gt; Type=GLX | WGL | WGLn | WGLa | CGL
* GPU&lt;integer&gt; Port=&lt;integer&gt; // X11 display number, 0 otherwise
* GPU&lt;integer&gt; Device=&lt;integer&gt; // X11 screen number, wglEnumGpusNV index, CGDirectDisplayID
* GPU&lt;integer&gt; Width=&lt;integer&gt;
* GPU&lt;integer&gt; Height=&lt;integer&gt;
* GPU&lt;integer&gt; X=&lt;integer&gt;
* GPU&lt;integer&gt; Y=&lt;integer&gt;

## Downloads

Version 1.0.3
[release notes](http://www.equalizergraphics.com/gpu-sd/API-1.0.3):

* Source: [tar.gz](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd-1.0.3.tar.gz)
* Binaries:
  [![OS X](http://www.equalizergraphics.com/images/mac.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.3-Darwin.dmg)
  [![Ubuntu](http://www.equalizergraphics.com/images/ubuntu.png)](https://launchpad.net/%7Eeilemann/+archive/equalizer/)
  [![Windows 32 bit](http://www.equalizergraphics.com/images/windows32.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.3-win32.exe)
  [![Windows 64 bit](http://www.equalizergraphics.com/images/windows64.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.3-win64.exe)

Version 1.0.2
[release notes](http://www.equalizergraphics.com/gpu-sd/API-1.0.2):

* Source: [tar.gz](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd-1.0.2.tar.gz)
* Binaries:
  [![OS X](http://www.equalizergraphics.com/images/mac.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.2-Darwin.dmg)
  [![Ubuntu](http://www.equalizergraphics.com/images/ubuntu.png)](https://launchpad.net/%7Eeilemann/+archive/equalizer/)
  [![Windows 32 bit](http://www.equalizergraphics.com/images/windows32.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.2-win32.exe)
  [![Windows 64 bit](http://www.equalizergraphics.com/images/windows64.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.2-win64.exe)

Version 1.0.1
[release notes](http://www.equalizergraphics.com/gpu-sd/API-1.0.1):

* Source: [tar.gz](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd-1.0.1.tar.gz)
* Binaries:
  [![OS X](http://www.equalizergraphics.com/images/mac.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.1-Darwin.dmg)
  [![Ubuntu](http://www.equalizergraphics.com/images/ubuntu.png)](https://launchpad.net/%7Eeilemann/+archive/equalizer/)
  [![Windows 32 bit](http://www.equalizergraphics.com/images/windows32.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.1-win32.exe)
  [![Windows 64 bit](http://www.equalizergraphics.com/images/windows64.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.1-win64.exe)

Version 1.0
[release notes](http://www.equalizergraphics.com/gpu-sd/API-1.0):

* Source: [tar.gz](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd-1.0.tar.gz)
* Binaries:
  [![OS X](http://www.equalizergraphics.com/images/mac.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.0-Darwin.dmg)
  [![Ubuntu](http://www.equalizergraphics.com/images/ubuntu.png)](https://launchpad.net/%7Eeilemann/+archive/equalizer/)
  [![Windows 32 bit](http://www.equalizergraphics.com/images/windows32.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.0-win32.exe)
  [![Windows 64 bit](http://www.equalizergraphics.com/images/windows64.png)](http://www.equalizergraphics.com/gpu-sd/downloads/gpu-sd1-1.0.0-win64.exe)

## Compilation

The build system is using CMake, with a default Makefile to trigger
CMake and compilation. Typing 'make' should suffice. A ZeroConf
implementation is required for the dns_sd module and the daemon. On Mac
OS X it is part of the operating system, on Linux AVAHI is tested ('sudo
apt-get install libavahi-compat-libdnssd-dev' on Ubuntu). If no ZeroConf
implementation is found, GPU-SD is only compiled with local discovery
modules.

## Usage

An application can use the discovery by linking the relevant module
libraries, instantiation the modules in the code and then quering the
instantiated modules. The following will find all remote and the local
GPUs on Windows:

    gpusd::wgl::Module::use();
    gpusd::dns_sd::Module::use();
    const gpusd::GPUInfos& infos = gpusd::Module::discoverGPUs();

Filters are chainable functors which can be passed to the query function
to discard information. The following filters are provided:

* DuplicateFilter eliminates duplicates, e.g.,  when one announcement is
  seen on multiple interfaces
* MirrorFilter eliminates the same GPU with a different type, e.g., when
  enabling both the cgl and glx module on Mac OS X.
* SessionFilter discards all GPUs not belonging to a given session

## Projects using GPU-SD

* [Equalizer](http://www.equalizergraphics.com) parallel rendering
  framework. ([source](https://github.com/Eyescale/Equalizer/blob/master/libs/eq/server/config/resources.cpp#L61))

## TODO List

* GPU&lt;integer&gt; Vendor=&lt;OpenGL vendor string&gt;
* GPU&lt;integer&gt; Renderer=&lt;OpenGL renderer string&gt;
* GPU&lt;integer&gt; Version=&lt;OpenGL version string&gt;
* GPU&lt;integer&gt; Segment Count=&lt;integer&gt;
* GPU&lt;integer&gt; Segment&lt;integer&gt; X=&lt;integer&gt;
* GPU&lt;integer&gt; Segment&lt;integer&gt; Y=&lt;integer&gt;
* GPU&lt;integer&gt; Segment&lt;integer&gt; Width=&lt;integer&gt;
* GPU&lt;integer&gt; Segment&lt;integer&gt; Height=&lt;integer&gt;
