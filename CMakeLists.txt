CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(ParaView_DIR /Users/jjm390/Development/paraview/build/ParaView-build)

FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(vtkMultiPortDemo "1.0"
  SERVER_MANAGER_XML vtkMultiPortDemo.xml
  SERVER_MANAGER_SOURCES vtkMultiPortDemo.cxx
  GUI_RESOURCE_FILES vtkMultiPortDemoGUI.xml)

INCLUDE_DIRECTORIES(${ParaView_INCLUDE_DIR})
Target_LINK_LIBRARIES(vtkMultiPortDemo ${ParaView_LIBRARIES} )

