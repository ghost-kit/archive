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
    scInfoProp.h
    )


# create a paraview plugin containing server manager xml and the server
# manager classes to build
# this plugin can be loaded on the server side


set(outifaces0)
set(outsrcs0)
ADD_PARAVIEW_PROPERTY_WIDGET(outifaces0 outsrcs0
        TYPE "SpaceCraftInfoProperties"
        CLASS_NAME "scInfoProp")


ADD_PARAVIEW_PLUGIN(SpaceCraftInfoSMPlugin "1.0"
	SERVER_MANAGER_XML SpaceCraftInfo.xml
        GUI_INTERFACES ${outifaces0}
        SERVER_MANAGER_SOURCES vtkSpaceCraftInfo.cxx
        SOURCES filterNetworkAccessModule.cpp
                scInfoProp.cpp
                ${outsrcs0}
                ${MOC_BUILT_SOURCES})


#Properties Panel

