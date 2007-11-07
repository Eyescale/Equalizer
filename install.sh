INSTALL_DIR=/usr/local
BUILD_DIR=build/`uname`

mkdir -p $DESTDIR/etc/ld.so.conf.d; echo $INSTALL_DIR/lib >> $DESTDIR/etc/ld.so.conf.d/Equalizer.conf
mkdir -p $DESTDIR/$INSTALL_DIR/bin; cp -r $BUILD_DIR/bin/ $DESTDIR/$INSTALL_DIR/bin
mkdir -p $DESTDIR/$INSTALL_DIR/include; cp -r $BUILD_DIR/include/ $DESTDIR/$INSTALL_DIR/include
mkdir -p $DESTDIR/$INSTALL_DIR/lib; cp -r $BUILD_DIR/lib/ $DESTDIR/$INSTALL_DIR/lib
mkdir -p $DESTDIR/$INSTALL_DIR/share; cp -r $BUILD_DIR/share/ $DESTDIR/$INSTALL_DIR/share
