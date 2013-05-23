# Equalizer Parallel Rendering Framework

Welcome to Equalizer, the standard middleware to create and deploy
parallel, scalable OpenGL applications. This release introduces major
new features, most notably asynchronous readbacks, region of interest
and thread affinity for increased performance during scalable rendering.

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
