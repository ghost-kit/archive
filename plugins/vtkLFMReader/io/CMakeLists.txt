cmake_minimum_required(VERSION 2.8)

#FIXME:  Use "find_package" for A++, MPI and Hdf4 rather than these options:
option (USE_APP "Use A++ and/or P++ for array storage" OFF)
option (USE_MPI "Build parallel I/O (Warning: leave this off, it's untested)" OFF)
option (HAS_HDF4 "Build HDF-4 Interface" ON)

if (USE_APP)
  add_definitions(-DBUILD_WITH_APP)
endif (USE_APP)

if (USE_MPI)
  add_definitions(-DBUILD_WITH_MPI)
endif (USE_MPI)

if (HAS_HDF4)
  add_definitions(-DHAS_HDF4)
endif (HAS_HDF4)

find_package(Hdf5)
if (HDF5_FOUND)
  INCLUDE_DIRECTORIES( ${HDF5_C_INCLUDE_DIR} )
  set( HDF5_LIBRARIES ${HDF5_hdf5_LIBRARY} ${HDF5_m_LIBRARY} ${HDF5_z_LIBRARY} )
  add_definitions(-DHAS_HDF5)
  if (HDF5_IS_PARALLEL)
    add_definitions(-DHAS_PHDF5)
  endif (HDF5_IS_PARALLEL)
endif (HDF5_FOUND)

add_library(io STATIC Io.hpp Io.C
            IoTemplates.hpp Util.hpp
            Hdf.hpp Hdf.C Hdf4.hpp Hdf4.C
            Hdf5.hpp Hdf5.C
            PHdf5.hpp PHdf5.C)

