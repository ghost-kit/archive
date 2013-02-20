CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

set(ParaView_DIR /Users/schmitt/paraview/opt/ParaView-3.98.1_OSX-10.7/paraview/src/paraview-build)
set(ParaVIew_SuperBuild_DIR /Users/schmitt/paraview/opt/src/ParaViewSuperbuild-3.98.1)
#set(ParaView_DIR /Users/schmitt/paraview/opt/ParaView-3.98.0_OSX-10.7/paraview/src/paraview-build)
#set(ParaVIew_SuperBuild_DIR /Users/schmitt/paraview/opt/src/ParaViewSuperbuild-3.98.0)

#SET(HDF4_DIR /Users/schmitt/paraview/opt/hdf-4.2.5-mac-intel-x86_64)
SET(PKG_DIR /Users/schmitt/paraview/opt)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})
ADD_PARAVIEW_PLUGIN(vtkLFMReader "1.0"
  SERVER_MANAGER_XML vtkLFMReader.xml
  SERVER_MANAGER_SOURCES vtkLFMReader.cxx 
  SOURCES Hdf4.C
  GUI_RESOURCE_FILES vtkLFMGUI.xml)

INCLUDE_DIRECTORIES(${PKG_DIR}/include)
LINK_DIRECTORIES(${PKG_DIR}/lib)

FIND_LIBRARY(LIB_DF    df    ${PKG_DIR}/lib)
FIND_LIBRARY(LIB_JPEG  jpeg ${PKG_DIR}/lib)
FIND_LIBRARY(LIB_MFHDF mfhdf ${PKG_DIR}/lib)
#FIND_LIBRARY(LIB_SZ  sz)
#FIND_LIBRARY(LIB_Z  z)


Target_LINK_LIBRARIES(vtkLFMReader 
  ${LIB_DF}
  ${LIB_JPEG}
  ${LIB_MFHDF}
#  ${LIB_Z}
)


add_custom_target(dist
    COMMAND ${CMAKE_COMMAND}  -DBINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -DPV_SUPERBUILD_LIST_DIR:PATH=${ParaVIew_SuperBuild_DIR}/Projects
                              -DPARAVIEW_BINARY_DIR:PATH=${ParaView_DIR}
 			      -DQT_LIBRARY_DIR:PATH=${QT_LIBRARY_DIR}
                              -DTMP_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -Dbundle_name:STRING=${CMAKE_CURRENT_BINARY_DIR}/vtkLFMReader.tgz
                              -P ${CMAKE_CURRENT_LIST_DIR}/install_vtkLFMReader.cmake
)
