
#fixup the plugin
if (APPLE)
  message(WARNING "Fixing up For Apple")
  set (SHARED_LIBRARY_PREFIX "lib")
  set (SHARED_LIBRARY_SUFFIX ".dylib")
elseif (UNIX)
  set (SHARED_LIBRARY_PREFIX "lib")
  set (SHARED_LIBRARY_SUFFIX ".so")
elseif(WIN32)
  message(FATAL_ERROR "Not supported on Windows")
endif()


# Remove any old directory.
  message(WARNING "Removing Old Directories")

execute_process(

  COMMAND ${CMAKE_COMMAND} -E remove_directory ${TMP_DIR}/EnlilToolsPlugin
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${TMP_DIR}}
  WORKING_DIRECTORY ${TMP_DIR}
)

# Create a temp directory to put the plugin under.
  message(WARNING "Creating New Directories")

execute_process(

    COMMAND ${CMAKE_COMMAND} -E make_directory ${TMP_DIR}
    WORKING_DIRECTORY ${TMP_DIR}
)

# Create a directory to put the plugin under.
  message(WARNING "Creating New Directories")

execute_process(

    COMMAND ${CMAKE_COMMAND} -E make_directory ${TMP_DIR}/EnlilToolsPlugin
    WORKING_DIRECTORY ${TMP_DIR}
)

# Copy the plugin lib.
  message(WARNING "Copying the Plugin to temp location")

execute_process(

  COMMAND ${CMAKE_COMMAND} -E copy ${BINARY_DIR}/libs/${SHARED_LIBRARY_PREFIX}vtkEnlilReader${SHARED_LIBRARY_SUFFIX} ${TMP_DIR}/EnlilToolsPlugin
  WORKING_DIRECTORY ${TMP_DIR}
)

if (APPLE)
  message(WARNING "Fixing up the Plugin")

  execute_process(
    COMMAND ${PV_SUPERBUILD_LIST_DIR}/apple/fixup_plugin.py
            # The directory containing the plugin dylibs or the plugin itself.
            ${TMP_DIR}/EnlilToolsPlugin/${SHARED_LIBRARY_PREFIX}vtkEnlilReader${SHARED_LIBRARY_SUFFIX}
            # names to replace (in order)
            "${PARAVIEW_BINARY_DIR}/lib/=@executable_path/../Libraries/"
            "${INSTALL_DIR}/lib/Qt=@executable_path/../Frameworks/Qt"
            "${INSTALL_DIR}/lib/=@executable_path/../Libraries/"
            "libhdf5.7.3.0.dylib=@executable_path/../Libraries/libhdf5.1.8.9.dylib"
            "libhdf5_hl.7.3.0.dylib=@executable_path/../Libraries/libhdf5.1.8.9.dylib"
            "libcgns.3.1.dylib=@executable_path/../Libraries/libcgns.3.1.dylib"
            )
endif()

  message(WARNING "Creating tarball")
execute_process(

    COMMAND ${CMAKE_COMMAND} -E tar cvz ${bundle_name} vtkEnlilReader
    WORKING_DIRECTORY ${TMP_DIR}
)
