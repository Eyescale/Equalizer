INSTALL_DIR=/usr/local
BUILD_DIR=build/`uname`
INSTALL_CMD="rsync -avx --exclude .svn"

mkdir -p $DESTDIR/etc/ld.so.conf.d
echo $INSTALL_DIR/lib >> $DESTDIR/etc/ld.so.conf.d/Equalizer.conf

mkdir -p $DESTDIR/$INSTALL_DIR/bin
$INSTALL_CMD $BUILD_DIR/bin/ $DESTDIR/$INSTALL_DIR/bin

mkdir -p $DESTDIR/$INSTALL_DIR/include
$INSTALL_CMD $BUILD_DIR/include/ $DESTDIR/$INSTALL_DIR/include

mkdir -p $DESTDIR/$INSTALL_DIR/lib
$INSTALL_CMD $BUILD_DIR/lib/ $DESTDIR/$INSTALL_DIR/lib

mkdir -p $DESTDIR/$INSTALL_DIR/share
$INSTALL_CMD $BUILD_DIR/share/ $DESTDIR/$INSTALL_DIR/share
