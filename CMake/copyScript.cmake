# Copyright (c) 2010 Stefan Eilemann <eile@eyescale.ch>

if(WIN32)
    if(NOT COPY_TARGET)
        set(COPY_TARGET "c:\\Equalizer" CACHE STRING "Target directory for the copyScript.bat")
    endif(NOT COPY_TARGET)
    if(NOT COPY_TARGET_RELEASE_TYPE)
        set(COPY_TARGET_RELEASE_TYPE Release CACHE STRING "Release configuration to copy (Release, RelWithDebInfo, MinSizeRel)")
    endif(NOT COPY_TARGET_RELEASE_TYPE)

    if(NOT CMAKE_GENERATOR)
        message(FATAL_ERROR "CMAKE_GENERATOR not set")
    endif()
    if("${CMAKE_GENERATOR}" MATCHES "Win64")
        set(PLATFORM_TYPE "x64\\")
    else()
        set(PLATFORM_TYPE "Win32\\")
    endif()
    
    set(WIN_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} WIN_BIN_DIR)
    
    set(WIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    FILE(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} WIN_INSTALL_DIR)
    
    set(WIN_SOURCE_DIR ${CMAKE_SOURCE_DIR})
    FILE(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR} WIN_SOURCE_DIR)
    
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/copyScript.bat
        "xcopy /s /y \"${WIN_INSTALL_DIR}\\include\\\"* \"${COPY_TARGET}\\include\"\n\n"
        
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\eqServer.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\eqServer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\netperf.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\netperf.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\affinityCheck.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\affinityCheck.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n\n"

        "copy \"${WIN_BIN_DIR}\\bin\\debug\\Equalizer.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\Equalizer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\libs\\client\\debug\\Equalizer.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}debug\"\n\n"

        "copy \"${WIN_BIN_DIR}\\bin\\debug\\EqualizerServer.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\EqualizerServer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\libs\\server\\debug\\EqualizerServer.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}debug\"\n\n"

        "copy \"${WIN_BIN_DIR}\\bin\\debug\\EqualizerAdmin.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\EqualizerAdmin.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\libs\\admin\\debug\\EqualizerAdmin.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}debug\"\n\n"
        
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\pthread.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\Debug\\pthread.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}debug\"\n\n"

        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\eqServer.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\netperf.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\affinityCheck.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n\n"

        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\Equalizer.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\libs\\client\\${COPY_TARGET_RELEASE_TYPE}\\Equalizer.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\EqualizerServer.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\libs\\server\\${COPY_TARGET_RELEASE_TYPE}\\EqualizerServer.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\EqualizerAdmin.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\libs\\admin\\${COPY_TARGET_RELEASE_TYPE}\\EqualizerAdmin.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}release\"\n\n"
    )

  if(COPY_TARGET_RELEASE_TYPE MATCHES "RelWithDebInfo")
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/copyScript.bat
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\eqServer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\netperf.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\affinityCheck.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\Equalizer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\EqualizerServer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\EqualizerAdmin.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\${COPY_TARGET_RELEASE_TYPE}\\pthread.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\${COPY_TARGET_RELEASE_TYPE}\\pthread.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}release\"\n\n"
    )                
  endif(COPY_TARGET_RELEASE_TYPE MATCHES "RelWithDebInfo")

    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/copyScript.bat
        "xcopy /s /y \"${WIN_SOURCE_DIR}\\libs\\server\\\"*.h \"${COPY_TARGET}\\include\\eq\\server\"\n"
        "xcopy /s /y \"${WIN_SOURCE_DIR}\\libs\\server\\equalizers\\\"*.h \"${COPY_TARGET}\\include\\eq\\server\\equalizers\"\n"
        
        "\"%ProgramFiles%\\TortoiseSVN\\bin\\SubWCRev.exe\" \"${WIN_SOURCE_DIR}\" > \"${COPY_TARGET}\\include\\eq\\Version.txt\"\n"
        "pause\n"
    )
endif(WIN32)
