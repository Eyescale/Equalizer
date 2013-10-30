# CMake Modules

## Use in other projects

First integration into another project:

    git remote add -f CMake https://github.com/Eyescale/CMake.git
    git read-tree --prefix=CMake -u CMake/master
    git commit -am 'Merging CMake subtree'

Setup for new clone of a project:

    git remote add -f CMake https://github.com/Eyescale/CMake.git

Update:

    git pull -s subtree CMake master
    git push

## Updates

    [fork repository]
    git clone https://github.com/<fork>/CMake.git
    [change]
    git commit ...
    git push
    [open pull request]

## Documentation

- **DoxygenRule**: *doxygen* target to build documentation into
    CMAKE_BINARY_DIR/doc. Optional *github* target to copy result to
    ../GITHUB_ORGANIZATION/Project-M.m/.
- **GNUModules**: *module* target to create a
    [GNU module](http://modules.sourceforge.net/). See file for details.
- [GitTargets documentation](doc/GitTargets.md)
- **UpdateFile**: *update_file* CMake function which uses configure_file
    but leaves target untouched if unchanged.
