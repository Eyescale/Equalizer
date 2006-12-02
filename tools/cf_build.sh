#!/bin/sh

# Automatic build script for Equalizer. Used on the sf compile farm and other
# machines. Note the hardcoded build directory below.
# Usage : <name> <email>

BASE=${HOME}/Software/build/equalizer

export PATH=${PATH}:/sw/bin
export DYLD_LIBRARY_PATH=${BASE}/build/`uname`/lib:${BASE}/build/`uname`/`uname -m`/lib
cd ${BASE}
exec 1>`hostname`.make
exec 2>&1
svn up
make clean || mail -s "Equalizer build failure [make clean] on `hostname`" $1 < `hostname`.make
make || mail -s "Equalizer build failure [make] on `hostname`" $1 < `hostname`.make
