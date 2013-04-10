cmake_minimum_required(VERSION 2.8)

option (USE_APP "Use A++ and/or P++ for array storage" OFF)
option (USE_MPI "Build parallel I/O (Warning: leave this off, it's untested)" OFF)
option (HAS_HDF4 "Build HDF-4 Interface" ON)
option (HAS_HDF5 "Build HDF-5 Interface" OFF)
option (HAS_PHDF5 "Build parallel HDF-5 Interface (requires USE_MPI=ON)" OFF)

#FIXME:  Add "FIND_HDF4" to set include/lib paths for Hdf4
#FIXME:  Add "FIND_HDF5" to set include/lib paths for Hdf5
#FIXME:  Add "FIND_PHDF5" to set include/lib paths for Parallel Hdf5

if (USE_APP)
  add_definitions(-DBUILD_WITH_APP)
endif (USE_APP)

if (USE_MPI)
  add_definitions(-DBUILD_WITH_MPI)
endif (USE_MPI)

if (HAS_HDF4)
  add_definitions(-DHAS_HDF4)
endif (HAS_HDF4)

if (HAS_HDF5)
  add_definitions(-DHAS_HDF5)
endif (HAS_HDF5)

if (HAS_PHDF5)
  add_definitions(-DHAS_PHDF5)
endif (HAS_PHDF5)

add_library(io STATIC Io.hpp Io.C
            IoTemplates.hpp Util.hpp
            Hdf.hpp Hdf.C Hdf4.hpp Hdf4.C
            Hdf5.hpp Hdf5.C
            PHdf5.hpp PHdf5.C)

