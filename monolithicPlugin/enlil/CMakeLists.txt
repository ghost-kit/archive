CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT("Enlil Reader 2.0.1")

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

#SET(ParaView_DIR /Volumes/Data/Development/SwFT/superbuild/paraview/src/paraview-build)
SET(ParaView_SuperBuild_DIR /Volumes/Data/Development/SwFt/ParaViewSuperbuild/)

#ADD_SUBDIRECTORY(cxform-0.71)

#Find_Library(cxform REQUIRED)

#include_directories(cxform-0.71)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})


ADD_PARAVIEW_PLUGIN(vtkEnlilReader "2.0.1"
  SERVER_MANAGER_XML vtkEnlilReader.xml
  SERVER_MANAGER_SOURCES vtkEnlilReader.cxx
  SOURCES DateTime.C readerCache.cpp readerCacheManager.cpp
  GUI_RESOURCE_FILES vtkEnlilGUI.xml)

SET(KIT_LIBS  vtkNetCDF_cxx )

Target_LINK_LIBRARIES(vtkEnlilReader  ${ParaView_LIBRARIES} ${KIT_LIBS})


# Adding 'dist' Make target to deploy plugin that's compatible with
# KitWare release of ParaView for Mac OS-X.  Requires SuperBuild with
# GIT_TAG set accordingly in versions.cmake.
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND}  -DBINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -DPV_SUPERBUILD_LIST_DIR:PATH=${ParaView_SuperBuild_DIR}/Projects
                              -DPARAVIEW_BINARY_DIR:PATH=${ParaView_DIR}
 			      -DQT_LIBRARY_DIR:PATH=${QT_LIBRARY_DIR}
                              -DTMP_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -Dbundle_name:STRING=${CMAKE_CURRENT_BINARY_DIR}/vtkEnlilReader.tgz
                              -P ${CMAKE_CURRENT_LIST_DIR}/install_vtkEnlilReader.cmake
)
