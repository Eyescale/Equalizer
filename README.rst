# GPU Service Discovery
=======================

Introduction
------------

GPU-SD is a daemon and library for the discovery and announcement of
graphics processing units using ZeroConf. Its primary use case is to
allow auto-configuration of ad-hoc GPU clusters.

Modules
-------

The GPU-SD library uses modules which implement discovery using
different protocols. Each module is a separate library, which can be
selectively linked by applications, limiting dependencies. Currently
available are:

- DNS_SD: Remote ZeroConf (Bonjour) discovery for GPUs announced by the daemon
- CGL: Local discovery of Carbon displays (Mac OS X only)
- GLX: Local discovery of X11 servers and screens
- WGL: Local discovery of WGL_NV_gpu_affinity, WGL_AMD_gpu_association
  or Windows displays (Windows only)

Daemon
------

The daemon uses all available local modules to query local GPUs and
announces them using ZeroConf on the local network. The service type
name is "_gpu-sd". The dns_sd discovery module gathers the information
announced by all servers on the local network. The following protocol is
used by the daemon:

* Session=default|<string>
* GPU Count=<integer>
* GPU<integer> Type=GLX | WGL | WGLn | WGLa | CGL
* GPU<integer> Port=<integer> // X11 display number, 0 otherwise
* GPU<integer> Device=<integer> // X11 screen number, wglEnumGpusNV index, CGDirectDisplayID
* GPU<integer> Width=<integer>
* GPU<integer> Height=<integer>
* GPU<integer> X=<integer>
* GPU<integer> Y=<integer>

Compilation
-----------

The build system is using CMake, with a default Makefile to trigger
CMake and compilation. Typing 'make' should suffice. A ZeroConf
implementation is required for the dns_sd module and the daemon. On Mac
OS X it is part of the operating system, on Linux AVAHI is tested ('sudo
apt-get install libavahi-compat-libdnssd-dev' on Ubuntu). If no ZeroConf
implementation is found, GPU-SD is only compiled with local discovery
modules.

Usage
-----

An application can use the discovery by linking the relevant module
libraries, instantiation the modules in the code and then quering the
instantiated modules. The following will find all remote and the local
GPUs on Windows:

    gpusd::wgl::Module::use();
    gpusd::dns_sd::Module::use();
    const gpusd::GPUInfos& infos = gpusd::Module::discoverGPUs();

TODO List
---------

* GPU<integer> Vendor=<OpenGL vendor string>
* GPU<integer> Renderer=<OpenGL renderer string>
* GPU<integer> Version=<OpenGL version string>
