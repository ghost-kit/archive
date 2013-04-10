PROJECT(SpaceCraftInfo)

cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

if (NOT ParaView_BINARY_DIR)
  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})
endif()

include(ParaViewPlugins)


# wrap QT
QT4_WRAP_CPP(
    MOC_BUILT_SOURCES
    filterNetworkAccessModule.h
    )


# create a paraview plugin containing server manager xml and the server
# manager classes to build
# this plugin can be loaded on the server side

ADD_PARAVIEW_PLUGIN(SpaceCraftInfoSMPlugin "1.0"
	SERVER_MANAGER_XML SpaceCraftInfo.xml
        SERVER_MANAGER_SOURCES vtkSpaceCraftInfo.cxx
        SOURCES filterNetworkAccessModule.cpp
                ${MOC_BUILT_SOURCES})

