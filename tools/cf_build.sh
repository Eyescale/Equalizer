#!/bin/sh

# Automatic build script for Equalizer. Used on the sf compile farm and other
# machines. Note the hardcoded build directory below.
# Usage : <name> <email>

PATH=${PATH}:/sw/bin
cd $HOME/Software/build/equalizer
exec 1>`hostname`.make
exec 2>&1
svn up
make clean || mail -s "Equalizer build failure [make clean] on `hostname`" $1 < `hostname`.make
make || mail -s "Equalizer build failure [make] on `hostname`" $1 < `hostname`.make
