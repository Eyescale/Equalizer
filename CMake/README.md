# CMake Modules

## Howto use in other projects

First integration into another project:

    git remote add -f CMake https://github.com/Eyescale/CMake.git
    git read-tree --prefix=CMake -u CMake/master
    git commit -am 'Merging CMake subtree'

Setup for new clone of a project:

    git remote add -f CMake https://github.com/Eyescale/CMake.git

Update:

    git pull -s subtree CMake master
    git push

## Documentation

- [GitTargets.cmake](CMake/blob/master/doc/GitTargets.md)
