# CMake Modules

Integration into another project:

    git remote add -f CMake https://github.com/Eyescale/CMake.git
    rm -rf CMake/*
    git checkout -- CMake/CPackConfig.cmake CMake/Equalizer.in.spec CMake/Equalizer.spec CMake/GPUSD.cmake CMake/configure.cmake CMake/copyScript.cmake CMake/stat.pl
    git commit -am 'Removing old, common CMake files'
    git read-tree --prefix=CMake -u CMake/master
    git commit -am 'Merging CMake subtree'

Update:

    git pull -s subtree CMake master
    git push
