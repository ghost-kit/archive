CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("Enlil Reader 2.0.1")

ADD_SUBDIRECTORY(cxform-0.71)

#Find_Library(cxform REQUIRED)

include_directories(cxform-0.71)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(vtkEnlilReader "2.0.1"
  SERVER_MANAGER_XML vtkEnlilReader.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  SOURCES DateTime.C
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)

SET(KIT_LIBS cxform vtkNetCDF_cxx vtkRenderingMatplotlib)

Target_LINK_LIBRARIES(vtkEnlilReader  ${ParaView_LIBRARIES} ${KIT_LIBS})


