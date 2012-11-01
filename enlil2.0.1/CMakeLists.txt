CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("Enlil Reader 2.0.1")

#SET(ParaView_DIR /Volumes/Data/Development/ParaView-src/git-version/build)
SET(ParaView_DIR /Volumes/Data/Development/ParaView/git-clean/build-13595/)

    set(CMAKE_CXX_FLAGS "-O3")        ## Optimize
    set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary

FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})

INCLUDE_DIRECTORIES(cxform-0.71)
ADD_SUBDIRECTORY(cxform-0.71)

ADD_PARAVIEW_PLUGIN(vtkEnlilReader "2.0.1"
  SERVER_MANAGER_XML vtkEnlilReader.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  SOURCES DateTime.C
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)

SET(KIT_LIBS vtkNetCDF_cxx cxform)

Target_LINK_LIBRARIES(vtkEnlilReader vtkRenderingMatplotlib ${ParaView_LIBRARIES} ${KIT_LIBS})

