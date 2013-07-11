PROJECT(CDAweb_Tool_Kit)
cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)

##########################################################################
# General Requirements:
#  * This plugin requires PARAVIEW v. 4.0.1 or LATER because of feature set used
#  * This plugin requires CMAKE 2.8.9 or later because ParaView Requires it
#  * This plugin utilizes BOOST.  if boost is not built into ParaView you must have access to the
#       BOOST static DATE_TIME library
#  * If building on Joshua Murphy's mac for distribution, use the -DBUILD_MAC=1 flag
#
# Required variables:
#  * ParaView_DIR: Path to directory containing ParaViewConfig.cmake
#  * ParaView_SuperBuild: Mac OS-X only:  SuperBuild is shipped with fixup_plugin.py, which is part of the SuperBuild.
#  * PREFIX:  Path to relevant libraries (HDF4, JPEG, etc)
#  * LIBCDF_LIBRARIES: path to static cdf library (libcdf.a) from the NASA cdf library (http://cdf.gsfc.nasa.gov/)
#  * LIBCDF_INCLUDE_DIRS: path to the include files from the above library (if not in the system include path)
#
# There are two ways to set required variables for building this plugin:
#  1.  Pass via command-line arguments (recommended):
#
#      cmake .. -DParaView_DIR=/Users/schmitt/paraview/opt/ParaView-3.98.1_OSX-10.7/paraview/src/paraview-build \
#               -DParaView_SuperBuild_DIR=/Users/schmitt/paraview/opt/src/ParaViewSuperbuild-3.98.0 \
#               -PREFIX=/Users/schmitt/paraview/opt \
#               -DLIBCDF_LIBRARIES=/usr/local/lib/libcdf.a
#               -DLIBCDF_INCLUDE_DIRS=/usr/local/include
#               -DBUILD_MAC=1
#
#  2.  Hard-code it in this file by uncommenting the following lines:

#set(ParaView_DIR /Volumes/Data/Development/ParaView/superbuild/paraview/src/paraview-build)
#set(ParaView_SuperBuild_DIR /Volumes/Data/Development/ParaView/ParaViewSuperbuild)
#set(LIBCDF_LIBRARIES /Applications/cdf34_1-dist/lib/libcdf.a)
#set(LIBCDF_INCLUDE_DIRS /Applications/cdf34_1-dist/include)

########################################################################

if( PREFIX )
  set( CMAKE_INCLUDE_PATH /usr/include ${PREFIX}/include ${CMAKE_INCLUDE_PATH} )
  set( CMAKE_LIBRARY_PATH /usr/lib ${PREFIX}/lib ${CMAKE_LIBRARY_PATH} )
endif( PREFIX )

#convienience funciton for ME. :)
if( BUILD_MAC )
  message(WARNING ": Your have selected to build on Joshua Murphy's Development Mac [At LASP], so I hope that is where you are!!!")
  message(STATUS "Changing your compilers Now!")
  set(CMAKE_C_COMPILER /Applications/Xcode-4.3.app/Contents/Developer/usr/bin/gcc)
  set(CMAKE_CXX_COMPILER /Applications/Xcode-4.3.app/Contents/Developer/usr/bin/g++)
  set(CMAKE_LIBRARY_PATH /Applications/Xcode-4.3.app/Contents/Developer/usr/lib)
  set(CMAKE_INCLUDE_PATH /Applications/Xcode-4.3.app/Contents/Developer/usr/include)
  set(CMAKE_OSX_ARCHITECTURES x86_64)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.6)
  set(CMAKE_OSX_SYSROOT /Applications/Xcode-4.3.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.6.sdk)
  set(ParaView_DIR /Volumes/Data/Development/ParaView/superbuild/paraview/src/paraview-build)
  set(ParaView_SuperBuild_DIR /Volumes/Data/Development/ParaView/ParaViewSuperbuild)
  set(LIBCDF_LIBRARIES /Applications/cdf34_1-dist/lib/libcdf.a)
  set(LIBCDF_INCLUDE_DIRS /Applications/cdf34_1-dist/include)
  message(STATUS ": Your Compilers are now:")
  message(STATUS "CXX: ${CMAKE_CXX_COMPILER}")
  message(STATUS "C:   ${CMAKE_C_COMPILER}")
  message(STATUS "You are now using the MAC OS X 10.6 SDK in Xcode-4.3")
  message(STATUS "Thank you for using Joshua Murphy's Mac.")

endif( BUILD_MAC )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/" )


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
ADD_PARAVIEW_PLUGIN(CDAWebTK "1.0.0.beta1"
        SERVER_MANAGER_XML ${SM_XML}
        GUI_INTERFACES ${PROP_ifaces}
        SERVER_MANAGER_SOURCES ${PLUGIN_SM_src}
        SOURCES ${PLUGIN_src}
                ${QT_Auto_src}
                ${PROP_src}
                )

target_link_libraries(CDAWebTK ${LIBS})



#for the mac distribution build
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND}  -DBINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -DPV_SUPERBUILD_LIST_DIR:PATH=${ParaView_SuperBuild_DIR}/Projects
                              -DPARAVIEW_BINARY_DIR:PATH=${ParaView_DIR}
                              -DQT_LIBRARY_DIR:PATH=${QT_LIBRARY_DIR}
                              -DTMP_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                              -Dbundle_name:STRING=${CMAKE_CURRENT_BINARY_DIR}/CDAwebTK.tgz
                              -P ${CMAKE_CURRENT_LIST_DIR}/install_CDAwebTK.cmake
)
