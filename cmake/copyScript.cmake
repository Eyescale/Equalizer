# Copyright (c) 2010 Daniel Pfeifer <daniel@pfeifer-mail.de>
#               2010 Stefan Eilemann <eile@eyescale.ch>

if(WIN32)
    if(NOT COPY_TARGET)
        set(COPY_TARGET "c:\\Equalizer" CACHE STRING "Target directory for the copyScript.bat")
    endif(NOT COPY_TARGET)
    if(CMAKE_SIZEOF_VOID_P MATCHES "8")
        set(PLATFORM_TYPE "x64\\")
    else(CMAKE_SIZEOF_VOID_P MATCHES "8")
        set(PLATFORM_TYPE "Win32\\")
    endif(CMAKE_SIZEOF_VOID_P MATCHES "8")
    
    set(WIN_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})
    FILE(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR} WIN_BIN_DIR)
    
    set(WIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
    FILE(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} WIN_INSTALL_DIR)
    
    set(WIN_SOURCE_DIR ${CMAKE_SOURCE_DIR})
    FILE(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR} WIN_SOURCE_DIR)
    
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/copyScript.bat
        "xcopy /s /y \"${WIN_INSTALL_DIR}\\include\\\"* \"${COPY_TARGET}\\include\"\n\n"
        
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\eqServer.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\netperf.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\affinityCheck.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\eqServer.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\netperf.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\affinityCheck.exe\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n\n"
        
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\eqServer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\netperf.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\affinityCheck.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\eqServer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\netperf.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\affinityCheck.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n\n"
        
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\Equalizer.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\Equalizer.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\Equalizer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\Equalizer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\lib\\debug\\Equalizer.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\lib\\release\\Equalizer.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}release\"\n\n"
        
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\EqualizerServer.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\EqualizerServer.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\EqualizerServer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\EqualizerServer.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\server\\debug\\EqualizerServer.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\server\\release\\EqualizerServer.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}release\"\n\n"
        
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\EqualizerAdmin.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\EqualizerAdmin.dll\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\debug\\EqualizerAdmin.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\bin\\release\\EqualizerAdmin.pdb\" \"${COPY_TARGET}\\bin\\${PLATFORM_TYPE}release\"\n"
        "copy \"${WIN_BIN_DIR}\\admin\\debug\\EqualizerAdmin.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}debug\"\n"
        "copy \"${WIN_BIN_DIR}\\admin\\release\\EqualizerAdmin.lib\" \"${COPY_TARGET}\\lib\\${PLATFORM_TYPE}release\"\n\n"
                
        "xcopy /s /y \"${WIN_SOURCE_DIR}\\server\\\"*.h \"${COPY_TARGET}\\include\\eq\\server\"\n"
        "xcopy /s /y \"${WIN_SOURCE_DIR}\\server\\equalizers\\\"*.h \"${COPY_TARGET}\\include\\eq\\server\\equalizers\"\n\n"
        
        "\"%ProgramFiles%\\TortoiseSVN\\bin\\SubWCRev.exe\" \"${WIN_SOURCE_DIR}\" > \"${COPY_TARGET}\\include\\eq\\Version.txt\"\n"
        "pause\n"
    )
endif(WIN32)
