PROJECT(SpaceCraftInfo)

cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)

#CMAKE Module Detection Made Easy...
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

#I am setting this from the cmake command line
if(NOT ParaView_BINARY_DIR)
  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})
endif()

#needed to build plugins
include(ParaViewPlugins)

#needed for these plugins
# At present I am specifying ${LIBCDF_INCLUDE_DIRS} and ${LIBCDF_LIBRARIES} from the cmake command line
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS DATE_TIME)
include_directories(${LIBCDF_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIBCDF_LIBRARIES} ${Boost_LIBRARIES})


# wrap QT
QT4_WRAP_CPP(
    MOC_BUILT_SOURCES
    filterNetworkAccessModule.h
    ScInfoPropWidget.h
    status.h
    filedownloader.h
    vtkSpaceCraftInfo.h
    pathSelector.h
    newFileChooserWidget.h
    )

# Wrap UI
QT4_WRAP_UI(
    UI_BUILT_SOURCES
    ScInfoPropWidget.ui
    status.ui
    pathSelector.ui
    )

# Property Widgets
set(outifaces0)
set(outsrcs0)
ADD_PARAVIEW_PROPERTY_WIDGET(outifaces0 outsrcs0
        TYPE "SpaceCraftInfoProperties"
        CLASS_NAME "ScInfoPropWidget")

set(outifaces1)
set(outsrcs1)
ADD_PARAVIEW_PROPERTY_WIDGET(outifaces1 outsrcs1
        TYPE "PathSelectionProperty"
        CLASS_NAME "pathSelector")

# Set up files for compilation
set(SM_XML SpaceCraftInfoFilter.xml SpaceCraftInfoSource.xml)
set(PROP_ifaces ${outifaces0} ${outifaces1})
set(PROP_src ${outsrcs0} ${outsrcs1})
set(QT_Auto_src ${MOC_BUILT_SOURCES} ${UI_BUILT_SOURCES})
set(PLUGIN_src filterNetworkAccessModule.cpp
                vtkSpaceCraftInfo.cxx
                ScInfoPropWidget.cpp
                status.cpp
                DateTime.C
                filedownloader.cpp
                timefithandler.cpp
                BadDataHandler.cpp
                cdfDataReader.cpp
                omitBDHandler.cpp
                pathSelector.cxx
                newFileChooserWidget.cxx)

set(PLUGIN_SM_src vtkSpaceCraftInfoFilter.cpp
                  vtkSpaceCraftInfoSource.cpp)


#plugin (Filter for CDAweb)
ADD_PARAVIEW_PLUGIN(CDAWebTK "1.0.beta1"
        SERVER_MANAGER_XML ${SM_XML}
        GUI_INTERFACES ${PROP_ifaces}
        SERVER_MANAGER_SOURCES ${PLUGIN_SM_src}
        SOURCES ${PLUGIN_src}
                ${QT_Auto_src}
                ${PROP_src}
                )

target_link_libraries(CDAWebTK ${LIBS})



