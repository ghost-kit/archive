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
    ScInfoPropWidget.h
    selectionBoxWidget.h
    )

# Wrap UI
QT4_WRAP_UI(
    UI_BUILT_SOURCES
    ScInfoPropWidget.ui
    selectionBoxWidget.ui
    )

# Property Widget
set(outifaces0)
set(outsrcs0)
ADD_PARAVIEW_PROPERTY_WIDGET(outifaces0 outsrcs0
        TYPE "SpaceCraftInfoProperties"
        CLASS_NAME "ScInfoPropWidget")


ADD_PARAVIEW_PLUGIN(SpaceCraftInfoSMPlugin "0.1.0"
	SERVER_MANAGER_XML SpaceCraftInfo.xml
        GUI_INTERFACES ${outifaces0}
        SERVER_MANAGER_SOURCES vtkSpaceCraftInfo.cxx
        SOURCES filterNetworkAccessModule.cpp
                ScInfoPropWidget.cpp
                selectionBoxWidget.cpp
                ${outsrcs0}
                ${MOC_BUILT_SOURCES}
                ${UI_BUILT_SOURCES}
                )

