CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("Enlil Reader 2.0.1")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")


ADD_SUBDIRECTORY(cxform-0.71)

#Find_Library(cxform REQUIRED)

include_directories(cxform-0.71)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})


ADD_PARAVIEW_PLUGIN(vtkEnlilReader "2.0.1"
  SERVER_MANAGER_XML vtkEnlilReader.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  SOURCES DateTime.C readerCache.cpp
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)

SET(KIT_LIBS cxform vtkNetCDF_cxx )

Target_LINK_LIBRARIES(vtkEnlilReader  ${ParaView_LIBRARIES} ${KIT_LIBS})



add_custom_target(dist
    COMMAND ${CMAKE_COMMAND}  -DBINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -DINSTALL_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -DPARAVIEW_BINARY_DIR:PATH=${ParaView_DIR}
                              -DTMP_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -Dbundle_name:STRING=${CMAKE_CURRENT_BINARY_DIR}/EnlilToolsPlugin.tgz
                              -P ${CMAKE_CURRENT_LIST_DIR}/install_plugin.cmake)
