CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(ParaView_DIR /Users/jjm390/Development/paraview/build/ParaView-build)

FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(vtkEnlilReader "1.0"
  SERVER_MANAGER_XML vtkEnlilReader.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)

ADD_PARAVIEW_PLUGIN(vtkEnlilReaderNonTime "1.0"
  SERVER_MANAGER_XML vtkEnlilReaderNonTime.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)


Target_LINK_LIBRARIES(vtkGenericReader ${ParaView_LIBRARIES})

