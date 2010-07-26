# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>

add_executable(eqHello hello.cpp)
target_link_libraries(eqHello Equalizer)
install(TARGETS eqHello DESTINATION bin)
