# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010-2011 Stefan Eilemann <eile@eyescale.ch>

set(EQPIXELBENCH_HEADERS channel.h config.h configEvent.h window.h)
set(EQPIXELBENCH_SOURCES channel.cpp config.cpp main.cpp window.cpp)
set(EQPIXELBENCH_LINK_LIBRARIES Equalizer)

common_application(eqPixelBench GUI EXAMPLE)

