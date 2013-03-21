
# Copyright (c) 2013 David Steiner <steiner@ifi.uzh.ch>

if (LUNCHBOX_FOUND)
  if (COLLAGE_FOUND)

    FIND_PACKAGE(Qt4)
    if(QT_FOUND)

      INCLUDE(${QT_USE_FILE})
      ADD_DEFINITIONS(${QT_DEFINITIONS})
      
      INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
      INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/ivs/src)
      
      set(QT_USE_OPENGL TRUE)
      
      set(visGUI_HEADERS
	connectDialog.h
	controller.h
	settings.h
	ivs/src/IO/File.h
	ivs/src/IO/TransferFunctionFile.h
	ivs/src/System/Config.h
	ivs/src/System/Destroyer.h
	ivs/src/System/Logger.h
	ivs/src/System/Messages.h
	ivs/src/System/OpenGL.h
	ivs/src/System/Timer.h
	ivs/src/System/Types.h
	ivs/src/TransferFunction.h
	ivs/src/TransferFunction.hh
	ivs/src/TransferFunctionFactory.h
	ivs/src/TransferFunctionGraph.h)
	
      set(visGUI_HEADERS_MOC
	mainWindow.h
	recentFiles.h
	ivs/src/TransferFunctionEditor.h
	ivs/src/TransferFunctionGraph.h)

      set(visGUI_SOURCES
	connectDialog.cpp
	controller.cpp
	main.cpp
	mainWindow.cpp
	recentFiles.cpp
	ivs/src/IO/File.cpp
	ivs/src/IO/TransferFunctionFile.cpp
	ivs/src/System/Logger.cpp
	ivs/src/System/Messages.cpp
	ivs/src/System/Timer.cpp
	ivs/src/TransferFunctionEditor.cpp
	ivs/src/TransferFunctionFactory.cpp
	ivs/src/TransferFunctionGraph.cpp
	ivs/src/TransferFunctionGraphCore.cpp)
	
      set(visGUI_RESOURCES
	resources/resources.qrc)
	
      set(visGUI_FORMS
	ivs/gui/TransferFunctionEditor.ui)
      
      QT4_WRAP_CPP(visGUI_SOURCES_MOC ${visGUI_HEADERS_MOC})
      QT4_WRAP_UI(visGUI_SOURCES_UI ${visGUI_FORMS})
      QT4_ADD_RESOURCES(visGUI_SOURCES_RRC ${visGUI_RESOURCES})
      
      massvol_add_tool(visGUI
	HEADERS
	  ${visGUI_HEADERS}
	  ${visGUI_HEADERS_MOC}
	SOURCES
	  ${visGUI_SOURCES}
	  ${visGUI_SOURCES_MOC}
	  ${visGUI_SOURCES_UI}
	  ${visGUI_SOURCES_RRC}
	LINK_LIBRARIES
	  ${LUNCHBOX_LIBRARIES}
	  ${COLLAGE_LIBRARIES}
	  ${QT_LIBRARIES}
	)
      
    else()
      message(STATUS "Qt4 is missing, omitting visGUI.")
    endif(QT_FOUND)
  else()
    message(WARNING "Collage is missing, omitting visGUI.")
  endif(COLLAGE_FOUND)
else()
  message(WARNING "Lunchbox is missing, omitting visGUI.")
endif(LUNCHBOX_FOUND)