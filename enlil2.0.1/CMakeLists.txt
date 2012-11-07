CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("Enlil Reader 2.0.1")

#SET(ParaView_DIR /Users/sysop/Developer/pv/build)
SET(ParaView_DIR /Volumes/Data/Development/ParaView-src/git-version/build-2Nov12)
#SET(ParaView_DIR /Volumes/Data/Development/ParaView/git-clean/build-13595/)

    set(CMAKE_CXX_FLAGS "-O3")        ## Optimize
    set(CMAKE_EXE_LINKER_FLAGS "-s")  ## Strip binary

ADD_SUBDIRECTORY(cxform-0.71)

Find_Library(cxform REQUIRED)

include_directories(cxform-0.71)

Add_Executable(cxtest cxform-0.71/main.c)
target_link_libraries(cxtest cxform)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})


ADD_PARAVIEW_PLUGIN(vtkEnlilReader "2.0.1"
  SERVER_MANAGER_XML vtkEnlilReader.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  SOURCES DateTime.C
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)

SET(KIT_LIBS cxform vtkNetCDF_cxx)

Target_LINK_LIBRARIES(vtkEnlilReader vtkRenderingMatplotlib ${ParaView_LIBRARIES} ${KIT_LIBS})

