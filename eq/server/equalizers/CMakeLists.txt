
set(HEADERS
    equalizer.h
    loadEqualizer.h
    tileEqualizer.h
    viewEqualizer.h
)

if(EQUALIZER_INSTALL_SERVER_HEADERS)
  install(FILES ${HEADERS} DESTINATION include/eq/server/equalizers
    COMPONENT dev)
endif()
