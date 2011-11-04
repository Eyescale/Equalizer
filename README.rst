Introduction
------------

GPU-SD is a daemon and library for the service discovery and
registration of graphics processing units using ZeroConf. Its primary
use case is to allow auto-configuration of ad-hoc GPU clusters.

Protocol
--------

The service type name is "_gpu-sd". The following text fields describe
the available GPUs:

* GPU Count=<integer>
* GPU<integer> Type=GLX | WGL | WGLn | CGL
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
implementation is required. On Mac OS X it is part of the operating
system, on Linux AVAHI is tested ('sudo apt-get install
libavahi-compat-libdnssd-dev' on Ubuntu).
