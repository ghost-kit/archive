cmake_minimum_required(VERSION 2.8)

option (USE_APP "Use A++ and/or P++ for array storage" OFF)
option (USE_MPI "Build parallel I/O (Warning: leave this off, it's untested)" OFF)
option (HAS_HDF4 "Build HDF-4 Interface" ON)
option (HAS_HDF5 "Build HDF-5 Interface" OFF)
option (HAS_PHDF5 "Build parallel HDF-5 Interface (requires USE_MPI=ON)" OFF)

#FIXME:  Add "FIND_HDF4" to set include/lib paths for Hdf4
#FIXME:  Add "FIND_HDF5" to set include/lib paths for Hdf5
#FIXME:  Add "FIND_PHDF5" to set include/lib paths for Parallel Hdf5

add_definitions(-DNOPP)
add_definitions(-DNOMPI)

if (USE_APP)
  remove_definitions(-DNOPP)
endif (USE_APP)

if (USE_MPI)
  remove_definitions(-DNOMPI)
endif (USE_MPI)

if (HAS_HDF4)
  SET( HDF4_SOURCES Hdf.C Hdf4.C )
endif (HAS_HDF4)

if (HAS_HDF5)
  SET( HDF5_SOURCES Hdf5.C )
endif (HAS_HDF5)

if (HAS_PHDF5)
  SET( PHDF5_SOURCES PHdf5.C )
endif (HAS_PHDF5)

add_library(io STATIC Io.C ${HDF4_SOURCES} ${HDF5_SOURCES} ${PHDF5_SOURCES})
