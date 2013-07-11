cmake_minimum_required(VERSION 2.8)

#FIXME:  Use "find_package" for A++, MPI and Hdf4 rather than these options:
option (USE_APP "Use A++ and/or P++ for array storage" OFF)
option (USE_MPI "Build parallel I/O (Warning: leave this off, it's untested)" OFF)

if (USE_APP)
  add_definitions(-DBUILD_WITH_APP)
endif (USE_APP)

if (USE_MPI)
  add_definitions(-DBUILD_WITH_MPI)
endif (USE_MPI)

########
# Hdf4 #
########################################################################
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/")
find_package(HDF4 REQUIRED)
if (HDF4_FOUND)
  find_package(JPEG)
  find_package(ZLIB)
  
  set(HDF4_LIBRARIES ${ZLIB_LIBRARY} ${JPEG_LIBRARY} ${HDF4_LIBRARIES})  

  add_definitions(-DHAS_HDF4)
endif (HDF4_FOUND)

########
# Hdf5 #
########################################################################
find_package(Hdf5)
if (HDF5_FOUND)
  set(HDF5_INCLUDE_DIRS ${HDF5_C_INCLUDE_DIR})
  set( HDF5_LIBRARIES ${HDF5_hdf5_LIBRARY} ${HDF5_m_LIBRARY} ${HDF5_z_LIBRARY} )
  add_definitions(-DHAS_HDF5)

  #################
  # Parallel Hdf5 #
  ########################################################################
  if (HDF5_IS_PARALLEL)
    add_definitions(-DHAS_PHDF5)
  endif (HDF5_IS_PARALLEL)
endif (HDF5_FOUND)

##############
# io library #
########################################################################
set( IO_INCLUDES ${HDF4_INCLUDE_DIR} ${HDF5_INCLUDE_DIRS} )
include_directories(${IO_INCLUDES})
set( IO_LIBRARIES ${HDF4_LIBRARIES} ${HDF5_LIBRARIES} ${PHDF5_LIBRARIES} )
add_library(io STATIC 
            ErrorQueue.h ErrorQueue.C
            Io.hpp Io.C
            IoTemplates.hpp Util.hpp
            Hdf.hpp Hdf.C Hdf4.hpp Hdf4.C
            Hdf5.hpp Hdf5.C
            PHdf5.hpp PHdf5.C)

###########
# cxxtest #
########################################################################
set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cxxtest )
find_package(CxxTest)
if(CXXTEST_FOUND)
  set(CXXTEST_USE_PYTHON TRUE)
  include_directories(${CXXTEST_INCLUDE_DIR})
  enable_testing()
  CXXTEST_ADD_TEST(testErrorQueue testErrorQueue.C "${CMAKE_CURRENT_SOURCE_DIR}/testErrorQueue.cxxTest")
  ADD_LIBRARY(test_ErrorQueue ErrorQueue.h ErrorQueue.C)
  TARGET_LINK_LIBRARIES(testErrorQueue test_ErrorQueue)
  CXXTEST_ADD_TEST(testNoppNompi testNoppNompi.C "${CMAKE_CURRENT_SOURCE_DIR}/testNoppNompi.cxxTest")
  target_link_libraries(testNoppNompi io ${IO_LIBRARIES} )
else ()
  message ( STATUS "Could NOT find CxxTest - unit testing not available" )
endif ()
