PROJECT(SPTK)
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
  set(SUPERBUILD=1)
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
# At present I am specifying ${CDF_INCLUDE_DIRS} and ${CDF_LIBRARIES} from the cmake command line

if(NOT SUPERBUILD)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    find_package(Boost COMPONENTS DATE_TIME REQUIRED)
    set(LIBS ${LIBS} ${Boost_LIBRARIES})
else(SUPERBUILD)
    set(LIBS ${LIBS} boost_date_time.a)
    message(STATUS "LIBS: ${LIBS}")
endif(NOT SUPERBUILD)

if(CDF_LIBRARIES)
    include_directories(${CDF_INCLUDE_DIRS})
    set(LIBS ${LIBS} ${CDF_LIBRARIES})
else(NOT CDF_LIBRARIES)
    message(FATAL_ERROR "CDF NOT FOUND!  Please provide CDF install information with CDF_LIBRARIES and CDF_INCLUDE_DIRS varaibles")
endif(CDF_LIBRARIES)

#set required include directories
INCLUDE_DIRECTORIES( CDAwebTK LFM enlil )

###########################################################
###################  CDA WEB TOOL KIT #####################
###########################################################
# wrap QT
QT4_WRAP_CPP(
    CDA_MOC_BUILT_SOURCES
    CDAwebTK/filterNetworkAccessModule.h
    CDAwebTK/ScInfoPropWidget.h
    CDAwebTK/status.h
    CDAwebTK/filedownloader.h
    CDAwebTK/vtkSpaceCraftInfo.h
    CDAwebTK/pathSelector.h
    CDAwebTK/newFileChooserWidget.h
    )

# Wrap UI
QT4_WRAP_UI(
    CDA_UI_BUILT_SOURCES
    CDAwebTK/ScInfoPropWidget.ui
    CDAwebTK/status.ui
    CDAwebTK/pathSelector.ui
    )

# Property Widgets
set(CDA_outifaces0)
set(CDA_outsrcs0)
ADD_PARAVIEW_PROPERTY_WIDGET(outifaces0 outsrcs0
        TYPE "SpaceCraftInfoProperties"
        CLASS_NAME "ScInfoPropWidget")

set(CDA_outifaces1)
set(CDA_outsrcs1)
ADD_PARAVIEW_PROPERTY_WIDGET(outifaces1 outsrcs1
        TYPE "PathSelectionProperty"
        CLASS_NAME "pathSelector")

# Set up files for compilation
set(CDA_SM_XML 	CDAwebTK/SpaceCraftInfoFilter.xml
			CDAwebTK/SpaceCraftInfoSource.xml)
			
set(CDA_PROP_ifaces ${CDA_outifaces0} ${CDA_outifaces1})
set(CDA_PROP_src ${CDA_outsrcs0} ${CDA_outsrcs1})
set(CDA_QT_Auto_src ${CDA_MOC_BUILT_SOURCES} ${CDA_UI_BUILT_SOURCES})
set(CDA_PLUGIN_src CDAwebTK/filterNetworkAccessModule.cpp
                CDAwebTK/vtkSpaceCraftInfo.cxx
                CDAwebTK/ScInfoPropWidget.cpp
                CDAwebTK/status.cpp
                CDAwebTK/DateTime.C
                CDAwebTK/filedownloader.cpp
                CDAwebTK/timefithandler.cpp
                CDAwebTK/BadDataHandler.cpp
                CDAwebTK/cdfDataReader.cpp
                CDAwebTK/omitBDHandler.cpp
                CDAwebTK/pathSelector.cxx
                CDAwebTK/newFileChooserWidget.cxx)

set(CDA_PLUGIN_SM_src CDAwebTK/vtkSpaceCraftInfoFilter.cpp
                  CDAwebTK/vtkSpaceCraftInfoSource.cpp)


###########################################################
##################  ENLIL READER PLUGIN ###################
###########################################################
add_subdirectory(enlil/cxform-0.71)
GET_DIRECTORY_PROPERTY(CXFORM_INCLUDES DIRECTORY enlil/cxform-0.71 DEFINITION CXFORM_INCLUDES)
GET_DIRECTORY_PROPERTY(CXFORM_LIBRARIES DIRECTORY enlil/cxform-0.71 DEFINITION CXFORM_LIBRARIES)
INCLUDE_DIRECTORIES(${CXFORM_INCLUDES})

set(LIBS ${LIBS} ${CXFORM_LIBRARIES} vtkNetCDF_cxx)

set(ENLIL_SM_XML enlil/vtkEnlilReader.xml)
set(ENLIL_UI_XML enlil/vtkEnlilGUI.xml)
set(ENLIL_SM_SRC enlil/vtkEnlilReader.cxx)
set(ENLIL_SRC    enlil/readerCache.cpp enlil/readerCacheManager.cpp)

###########################################################
###################  LFM READER PLUGIN ####################
###########################################################
add_subdirectory(LFM/io)
GET_DIRECTORY_PROPERTY(IO_INCLUDES DIRECTORY LFM/io DEFINITION IO_INCLUDES)
GET_DIRECTORY_PROPERTY(IO_LIBRARIES DIRECTORY LFM/io DEFINITION IO_LIBRARIES)
include_directories( ${IO_INCLUDES} )

set(LIBS ${LIBS} ${IO_LIBRARIES})

set(LFM_SM_XML LFM/vtkLFMReader.xml)
set(LFM_UI_XML LFM/vtkLFMGUI.xml)
set(LFM_SM_SRC LFM/vtkLFMReader.xml)


#plugin (Filter for CDAweb)
ADD_PARAVIEW_PLUGIN(SPTK "1.0.0.beta1"
        SERVER_MANAGER_XML ${CDA_SM_XML} ${ENLIL_SM_XML} ${LFM_SM_XML}
        GUI_RESOURCE_FILES ${ENLIL_UI_XML} ${LFM_UI_XML}
        GUI_INTERFACES ${CDA_PROP_ifaces}
        SERVER_MANAGER_SOURCES ${CDA_PLUGIN_SM_src} ${ENLIL_SM_SRC} ${LFM_SM_SRC}
        SOURCES ${CDA_PLUGIN_src}
                ${CDA_QT_Auto_src}
                ${CDA_PROP_src}
                ${ENLIL_SRC})

target_link_libraries(SPTK ${LIBS})



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
