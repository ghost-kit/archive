CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("Enlil Reader 2.0")

SET(ParaView_DIR /Users/jjm390/Development/paraview/build/ParaView-build)

FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(vtkEnlilReaderNonTime "2.0"
  SERVER_MANAGER_XML vtkEnlilReaderNonTime.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)

SET(KIT_LIBS vtkNetCDF_cxx)

Target_LINK_LIBRARIES(vtkEnlilReaderNonTime ${ParaView_LIBRARIES} ${KIT_LIBS})

