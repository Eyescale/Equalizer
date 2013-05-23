#std13

TEMPLATE = app

CONFIG += x86_64
CONFIG -= i386

EQ_INSTALL_DIR = $$(EQUALIZER_ROOT)
CO_INSTALL_DIR = $$(COLLAGE_ROOT)
LB_INSTALL_DIR = $$(LUNCHBOX_ROOT)
VL_INSTALL_DIR = $$(VMMLIB_ROOT)

DESTDIR     = $$EQ_INSTALL_DIR/bin
VPATH       = $$DESTDIR
OBJECTS_DIR = $$DESTDIR/qmake
MOC_DIR     = $$DESTDIR/moc
RCC_DIR     = ../resources

DEPENDPATH += .. \
              ../ivs/gui \
              ../ivs/src \
              ../ivs/src/GUI \
              ../ivs/src/IO \
              ../ivs/src/System

INCLUDEPATH += $$CO_INSTALL_DIR/include/  \
               $$LB_INSTALL_DIR/include/  \
               $$VL_INSTALL_DIR/include/  \
               ../../../../../libs/       \
               ../../../libs/             \
               ../ivs/src                 \
               ../../..

LIBS         = -L$$EQ_INSTALL_DIR/lib            \
               -L$$CO_INSTALL_DIR/lib -lCollage  \
               -L$$LB_INSTALL_DIR/lib -lLunchbox

UI_DIR = ../ivs/src/GUI/

CONFIG += opengl

# QMAKE_CXXFLAGS += -Wno-unknown-pragmas

RESOURCES += ../resources/resources.qrc

HEADERS = \
    ../connectDialog.h \
    ../controller.h \
    ../mainWindow.h \
    ../recentFiles.h \
    ../settings.h \
    ../ivs/src/IO/File.h \
    ../ivs/src/IO/TransferFunctionFile.h \
    ../ivs/src/System/Config.h \
    ../ivs/src/System/Destroyer.h \
    ../ivs/src/System/Logger.h \
    ../ivs/src/System/Messages.h \
    ../ivs/src/System/OpenGL.h \
    ../ivs/src/System/Timer.h \
    ../ivs/src/System/Types.h \
    ../ivs/src/TransferFunction.h \
    ../ivs/src/TransferFunction.hh \
    ../ivs/src/TransferFunctionEditor.h \
    ../ivs/src/TransferFunctionFactory.h \
    ../ivs/src/TransferFunctionGraph.h \
    ../ivs/src/TransferFunctionGraphCore.h


FORMS += ../ivs/gui/TransferFunctionEditor.ui

SOURCES = \
    ../connectDialog.cpp \
    ../controller.cpp \
    ../main.cpp \
    ../mainWindow.cpp \
    ../recentFiles.cpp \
    ../ivs/src/IO/File.cpp \
    ../ivs/src/IO/TransferFunctionFile.cpp \
    ../ivs/src/System/Logger.cpp \
    ../ivs/src/System/Messages.cpp \
    ../ivs/src/System/Timer.cpp \
    ../ivs/src/TransferFunctionEditor.cpp \
    ../ivs/src/TransferFunctionFactory.cpp \
    ../ivs/src/TransferFunctionGraph.cpp \
    ../ivs/src/TransferFunctionGraphCore.cpp
