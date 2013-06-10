# Try to find CDF library files

# Once done this will define
#  LIBCDF_FOUND - System has LibCDF
#  LIBCDF_INCLUDE_DIRS - The Libcdf include directories
#  LIBCDF_LIBRARIES - The libraries needed to use LibCDF
#  LIBCDF_DEFINITIONS - Compiler switches required for using LibCDF

#find_package(PkgConfig)
#pkg_check_modules(PC_LIBCDF QUIET libcdf)
#set(LIBCDF_DEFINITIONS ${PC_LIBCDF_CFLAGS_OTHER})

#find_path(LIBCDF_INCLUDE_DIR libcdf/xpath.h
#          HINTS ${PC_LIBCDF_INCLUDEDIR} ${PC_LIBCDF_INCLUDE_DIRS}
#          PATH_SUFFIXES libcdf )

#find_library(LIBCDF_LIBRARY NAMES cdf libcdf
#             HINTS ${PC_LIBCDF_LIBDIR} ${PC_LIBCDF_LIBRARY_DIRS} )

#set(LIBCDF_LIBRARIES ${LIBCDF_LIBRARY} )
#set(LIBCDF_INCLUDE_DIRS ${LIBCDF_INCLUDE_DIR} )

#include(FindPackageHandleStandardArgs)
## handle the QUIETLY and REQUIRED arguments and set LIBCDF_FOUND to TRUE
## if all listed variables are TRUE
#find_package_handle_standard_args(Libcdf  DEFAULT_MSG
#                                  LIBCDF_LIBRARY LIBCDF_INCLUDE_DIR)

#mark_as_advanced(LIBCDF_INCLUDE_DIR LIBCDF_LIBRARY )
