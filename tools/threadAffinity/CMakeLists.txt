# Copyright (c) 2016 Stefan.Eilemann@epfl.ch

set(EQTHREADAFFINITY_SOURCES threadAffinity.cpp)
set(EQTHREADAFFINITY_LINK_LIBRARIES Equalizer EqualizerServer)
common_application(eqThreadAffinity)
