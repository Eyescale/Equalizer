
1. Introduction
---------------

GPU-SD is a daemon and library for the service discovery and
registration of graphics processing units. Its primary use case is to
allow auto-configuration of ad-hoc GPU clusters.

2. Protocol
-----------

The service type name is "_gpu-sd". The following text fields describe
the available GPUs:

    GPU Count=<integer>
    GPU<integer> Type=X11 | WGL_NV_gpu_affinity | CG
    GPU<integer> Port=<integer> // X11 display number, 0 otherwise
    GPU<integer> Device=<integer> // X11 screen number, wglEnumGpusNV index, CGDirectDisplayID
    GPU<integer> Width=<integer>
    GPU<integer> Height=<integer>
    GPU<integer> X=<integer>
