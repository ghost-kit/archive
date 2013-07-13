#
# copyright : (c) 2010 Maxime Lenoir, Alain Coulais,
#                      Sylwester Arabas and Orion Poplawski
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
# https://github.com/cenit/GDL/blob/master/CMakeModules/FindHDF.cmake

# HDF4_LIBRARIES
# HDF4_EXTRA_LIBRARIES
find_package(JPEG)
find_library(HDF4_LIBRARY NAMES df dfalt PATH_SUFFIXES hdf)
find_library(MFHDF4_LIBRARY NAMES mfhdf mfhdfalt PATH_SUFFIXES hdf)
set(HDF4_LIBRARIES ${MFHDF4_LIBRARY} ${HDF4_LIBRARY})
find_path(HDF4_INCLUDE_DIR NAMES hdf.h PATH_SUFFIXES hdf)
include(CheckLibraryExists)
include(FindPackageHandleStandardArgs)
if(HDF4_LIBRARIES)
	set(CMAKE_REQUIRED_LIBRARIES z ${JPEG_LIBRARIES})
	check_library_exists("${HDF4_LIBRARIES}" Hopen "" HDF4_WO_SZIP)
	if(HDF4_WO_SZIP)
		set(HDF4_EXTRA_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
		find_package_handle_standard_args(HDF4 DEFAULT_MSG HDF4_LIBRARIES HDF4_EXTRA_LIBRARIES HDF4_INCLUDE_DIR)
	else(HDF4_WO_SZIP)
		find_library(SZIP_LIBRARIES NAMES sz szip)
		if(SZIP_LIBRARIES)
			set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${SZIP_LIBRARIES})
			check_library_exists("${HDF4_LIBRARIES}" Hopen "" HDF4_W_SZIP)	
			if(HDF4_W_SZIP)
				set(HDF4_EXTRA_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
				find_package_handle_standard_args(HDF4 DEFAULT_MSG HDF4_LIBRARIES HDF4_EXTRA_LIBRARIES HDF4_INCLUDE_DIR)
			endif(HDF4_W_SZIP)
		endif(SZIP_LIBRARIES)
	endif(HDF4_WO_SZIP)
	set(CMAKE_REQUIRED_LIBRARIES)
endif(HDF4_LIBRARIES)

mark_as_advanced(
HDF4_LIBRARY
MFHDF4_LIBRARY
HDF4_LIBRARIES
HDF4_INCLUDE_DIR
HDF4_WO_SZIP
HDF4_EXTRA_LIBRARIES
SZIP_LIBRARIES
HDF4_W_SZIP
)
