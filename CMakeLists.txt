CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(ParaView_DIR /Users/jjm390/Development/paraview/build/ParaView-build)

FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(vtkGenericReader "1.0"
  SERVER_MANAGER_XML vtkGenericReader.xml
  SERVER_MANAGER_SOURCES vtkGenericReader.cxx
  GUI_RESOURCE_FILES vtkGenericGUI.xml)

ADD_PARAVIEW_PLUGIN(vtkGenericReaderNonTime "1.0"
  SERVER_MANAGER_XML vtkGenericReaderNonTime.xml
  SERVER_MANAGER_SOURCES vtkGenericReader.cxx
  GUI_RESOURCE_FILES vtkGenericGUI.xml)


Target_LINK_LIBRARIES(vtkGenericReader ${ParaView_LIBRARIES})

