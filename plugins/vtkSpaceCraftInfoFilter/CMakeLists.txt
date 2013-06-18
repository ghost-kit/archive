PROJECT(SpaceCraftInfo)

cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

if(NOT ParaView_BINARY_DIR)
  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})
endif()

include(ParaViewPlugins)

#find_package(cdf REQUIRED)
include_directories(${LIBCDF_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIBCDF_LIBRARIES})


# wrap QT
QT4_WRAP_CPP(
    MOC_BUILT_SOURCES
    filterNetworkAccessModule.h
    ScInfoPropWidget.h
    statusbar.h
    status.h
    filedownloader.h
    vtkSpaceCraftInfo.h
    )

# Wrap UI
QT4_WRAP_UI(
    UI_BUILT_SOURCES
    ScInfoPropWidget.ui
    statusbar.ui
    status.ui
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
        SERVER_MANAGER_SOURCES vtkSpaceCraftInfoFilter.cpp
        SOURCES filterNetworkAccessModule.cpp
                vtkSpaceCraftInfo.cxx
                ScInfoPropWidget.cpp
                statusbar.cpp
                status.cpp
                DateTime.C
                filedownloader.cpp
                timefithandler.cpp
                BadDataHandler.cpp
                cdfDataReader.cpp
                omitBDHandler.cpp
                ${outsrcs0}
                ${MOC_BUILT_SOURCES}
                ${UI_BUILT_SOURCES}
                )

target_link_libraries(SpaceCraftInfoSMPlugin ${LIBS})


ADD_PARAVIEW_PLUGIN(SpaceCraftnInfoSource "0.1.0"
        SERVER_MANAGER_XML SpaceCraftInfoSource.xml
        GUI_INTERFACES ${outifaces0}
        SERVER_MANAGER_SOURCES vtkSpaceCraftInfoSource.cpp
        SOURCES filterNetworkAccessModule.cpp
                vtkSpaceCraftInfo.cxx
                ScInfoPropWidget.cpp
                statusbar.cpp
                status.cpp
                DateTime.C
                filedownloader.cpp
                timefithandler.cpp
                BadDataHandler.cpp
                cdfDataReader.cpp
                omitBDHandler.cpp
                ${outsrcs0}
                ${MOC_BUILT_SOURCES}
                ${UI_BUILT_SOURCES}
                )

target_link_libraries(SpaceCraftnInfoSource ${LIBS})

