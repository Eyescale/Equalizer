# Equalizer Parallel Rendering Framework

Welcome to Equalizer, the standard middleware to create and deploy
parallel, scalable OpenGL applications. It enables applications to
benefit from multiple graphics cards, processors and computers to scale
the rendering performance, visual quality and display size. An Equalizer
application runs unmodified on any visualization system, from a simple
workstation to large scale graphics clusters, multi-GPU workstations and
Virtual Reality installations.

# Building from source
## Linux, Mac OS X

```
  git clone https://github.com/Eyescale/Buildyard.git
  cd Buildyard
  git clone https://github.com/Eyescale/config.git config.eyescale
  make Equalizer
```

## Windows
```
  git clone https://github.com/Eyescale/Buildyard.git
  cd Buildyard
  git clone https://github.com/Eyescale/config.git config.eyescale
  [Use CMake UI to configure]
  [Open Buildyard.sln]
  [Build 00_Main - Equalizer]
```
