CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("Enlil Reader 2.0")

SET(ParaView_DIR /Volumes/Data/Development/ParaView-src/git-version/build)

    set(CMAKE_CXX_FLAGS "-O3")        ## Optimize
    set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary

FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})

ADD_PARAVIEW_PLUGIN(vtkEnlilReader "2.0"
  SERVER_MANAGER_XML vtkEnlilReader.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  SOURCES DateTime.C
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)


Target_LINK_LIBRARIES(vtkEnlilReader vtkNetCDF_cxx vtkRenderingMatplotlib ${ParaView_LIBRARIES})

