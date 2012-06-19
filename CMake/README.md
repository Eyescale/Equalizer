# CMake Modules

## Howto use in other projects

First integration into another project:

    git remote add -f CMake https://github.com/Eyescale/CMake.git
    rm -rf CMake/*
    git checkout -- CMake/CPackConfig.cmake CMake/Equalizer.in.spec CMake/Equalizer.spec CMake/GPUSD.cmake CMake/configure.cmake CMake/copyScript.cmake CMake/stat.pl
    git commit -am 'Removing old, common CMake files'
    git read-tree --prefix=CMake -u CMake/master
    git commit -am 'Merging CMake subtree'

Setup for new clone of a project:

    git remote add -f CMake https://github.com/Eyescale/CMake.git

Update:

    git pull -s subtree CMake master
    git push

## Documentation

- [GitTargets.cmake](CMake/blob/master/doc/GitTargets.md)
