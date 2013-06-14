CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Required variables:
#  * ParaView_DIR: Path to directory containing ParaViewConfig.cmake
#  * ParaView_SuperBuild: Mac OS-X only:  SuperBuild is shipped with fixup_plugin.py, which is part of the SuperBuild.
#  * PREFIX:  Path to relevant libraries (HDF4, JPEG, etc)
#
# There are two ways to set required variables for building this plugin:
#  1.  Pass via command-line arguments (recommended):
#  
#      cmake .. -DParaView_DIR=/Users/schmitt/paraview/opt/ParaView-3.98.1_OSX-10.7/paraview/src/paraview-build \
#               -DParaView_SuperBuild_DIR=/Users/schmitt/paraview/opt/src/ParaViewSuperbuild-3.98.0 \
#               -PREFIX=/Users/schmitt/paraview/opt
#  
#  2.  Hard-code it in this file by uncommenting the following lines:

#set(ParaView_DIR /Users/schmitt/paraview/opt/ParaView-3.98.1_OSX-10.7/paraview/src/paraview-build)
#set(ParaView_SuperBuild_DIR /Users/schmitt/paraview/opt/src/ParaViewSuperbuild-3.98.1)
#set(ParaView_DIR /Users/schmitt/paraview/opt/ParaView-3.98.0_OSX-10.7/paraview/src/paraview-build)
#set(ParaView_SuperBuild_DIR /Users/schmitt/paraview/opt/src/ParaViewSuperbuild-3.98.0)

########################################################################


if( PREFIX )
  set( CMAKE_INCLUDE_PATH /usr/include ${PREFIX}/include ${CMAKE_INCLUDE_PATH} )
  set( CMAKE_LIBRARY_PATH /usr/lib ${PREFIX}/lib ${CMAKE_LIBRARY_PATH} )
endif( PREFIX )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/" )

add_subdirectory(io) 
GET_DIRECTORY_PROPERTY(IO_INCLUDES DIRECTORY io DEFINITION IO_INCLUDES)
GET_DIRECTORY_PROPERTY(IO_LIBRARIES DIRECTORY io DEFINITION IO_LIBRARIES)
include_directories( ${IO_INCLUDES} )

FIND_PACKAGE(ParaView REQUIRED)

INCLUDE(${PARAVIEW_USE_FILE})
ADD_PARAVIEW_PLUGIN(vtkLFMReader "1.0"
  SERVER_MANAGER_XML vtkLFMReader.xml
  SERVER_MANAGER_SOURCES vtkLFMReader.cxx 
  GUI_RESOURCE_FILES vtkLFMGUI.xml)

Target_LINK_LIBRARIES(vtkLFMReader 
  io
  vtkhdf5
  ${IO_LIBRARIES}
)


add_custom_target(dist
    COMMAND ${CMAKE_COMMAND}  -DBINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -DPV_SUPERBUILD_LIST_DIR:PATH=${ParaView_SuperBuild_DIR}/Projects
                              -DPARAVIEW_BINARY_DIR:PATH=${ParaView_DIR}
 			      -DQT_LIBRARY_DIR:PATH=${QT_LIBRARY_DIR}
                              -DTMP_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -Dbundle_name:STRING=${CMAKE_CURRENT_BINARY_DIR}/vtkLFMReader.tgz
                              -P ${CMAKE_CURRENT_LIST_DIR}/install_vtkLFMReader.cmake
)
