cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)
project(SwFT)



FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})
if (NOT PARAVIEW_BUILD_QT_GUI)
  message(FATAL_ERROR
    "SwFT requires PARAVIEW_BUILD_QT_GUI to be enabled. "
    "Please rebuild ParaView (or point to a different build of ParaView) "
    "with PARAVIEW_BUILD_QT_GUI set to TRUE")
endif()


# include rules for creating custom applications.
include(ParaViewBranding)

#------------------------------------------------------------------------------
# Add extra library containing custom code for the client.

SET(swft_RESOURCES resources.qrc)
QT4_ADD_RESOURCES(swft_RESOURCES_RCC ${swft_RESOURCES})


QT4_WRAP_CPP(MOC_BUILT_SOURCES
  myMainWindow.h
  swftMainControlsToolbar.h
  swftMainControlsWidget.h
  swftMainPropertiesWidget.h
  swftControlsStackUnit.h
  swftCommandButton.h
  swftCommandSubButton.h
  swftStatusWindowWidget.h
    )

QT4_WRAP_UI(UI_BUILT_SOURCES
  myMainWindow.ui
  swftMainControlsToolbar.ui
  swftMainControlsWidget.ui
  swftMainPropertiesWidget.ui
  swftControlsStackUnit.ui
  swftCommandButton.ui
  swftCommandSubButton.ui
  swftStatusWindowWidget.ui
)

SOURCE_GROUP("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
)

SET(ParaView_SOURCE_FILES
  myMainWindow.cxx
  myMainWindow.h
  swftParaViewMenuBuilders.cpp
  swftParaViewMenuBuilders.h
  swftMainControlsToolbar.cxx
  swftMainControlsToolbar.h
 swftMainControlsWidget.h
 swftMainControlsWidget.cpp
 swftMainPropertiesWidget.cpp
 swftMainPropertiesWidget.h
 swftControlsStackUnit.cpp
 swftControlsStackUnit.h
 swftCommandButton.cpp
 swftCommandButton.h
 swftCommandSubButton.cpp
 swftCommandSubButton.h
 swftStatusWindowWidget.h
 swftStatusWindowWidget.cpp
  ${swft_RESOURCES_RCC}
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES})

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

build_paraview_client(SwFT
  TITLE "LASP Space weather Forecast Tools (SwFT)"
  ORGANIZATION  "LASP, CU Boulder"
  VERSION_MAJOR 0
  VERSION_MINOR 1
  VERSION_PATCH 1
  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/1.png"
  PVMAIN_WINDOW myMainWindow
  PVMAIN_WINDOW_INCLUDE myMainWindow.h
  GUI_CONFIGURATION_XMLS
    ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewSources.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewFilters.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewReaders.xml
    ${CMAKE_CURRENT_SOURCE_DIR}/ParaViewWriters.xml
  SOURCES ${ParaView_SOURCE_FILES}
)
