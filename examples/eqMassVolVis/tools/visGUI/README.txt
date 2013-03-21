

Running from Xcode requires exporting:

DYLD_LIBRARY_PATH /usr/local/cuda/lib:/Users/maxus/eq_install/lib




------------------------------------------------------------------------------------------------------------------------
EqPly application side:

EqPly.hpp                                    : Has the GUIDock as a member. The cpp doesn?t use the object as listening starts in its constructor, but
                                                       you might want to change the GUIDock object to have functions for initialization from command line, etc...
guiDock.h/cpp      : The listener node, which listens on (VOL_VIS_GUI_DEFAULT_HOST:DEFAULT_PORT) for the gui packets.
                                                       It is a member of EqPly class, and starts listening on constructor, so not even in eqPly.cpp. It might make sense to initialize it
                                                       with LocalInitData parameters, and make it listen through a start() method instead of the constructor after EqPly.cpp calls config->init
                                                       or something like that.
                                                       You see that there is a GUINode private class. In implementation of listen() of that class, it registers which function to call on which packet, which
                                                       you will expand when you want to add new packets to GUIPacket.hpp. In _cmdSetX method, it just prints out the x value communicated through the package,
                                                       but it would be best to give the Config object to the constructor, so it can alter the Config::_frameData in the _cmd* methods.

