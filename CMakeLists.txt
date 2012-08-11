CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("Enlil Reader 2.0")

SET(ParaView_DIR /home/jjm390/Development/paraview/build/ParaView-build)

    set(CMAKE_CXX_FLAGS "-O3")        ## Optimize
    set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary

FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(vtkEnlilReader "2.0"
  SERVER_MANAGER_XML vtkEnlilReader.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)

SET(KIT_LIBS vtkNetCDF_cxx)

Target_LINK_LIBRARIES(vtkEnlilReader ${ParaView_LIBRARIES} ${KIT_LIBS})

