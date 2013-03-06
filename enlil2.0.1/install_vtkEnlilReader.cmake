if (APPLE)
  set (SHARED_LIBRARY_PREFIX "lib")
  set (SHARED_LIBRARY_SUFFIX ".dylib")
  set (PLUGIN_DIR "lib")
elseif (UNIX)
  set (SHARED_LIBRARY_PREFIX "lib")
  set (SHARED_LIBRARY_SUFFIX ".so")
  set (PLUGIN_DIR "lib")
elseif (WIN32)
  set (SHARED_LIBRARY_PREFIX "")
  set (SHARED_LIBRARY_SUFFIX ".dll")
  set (PLUGIN_DIR "bin")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${TMP_DIR}/vtkEnlilReader)
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${TMP_DIR}/vtkEnlilReader)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${BINARY_DIR}/${SHARED_LIBRARY_PREFIX}vtkEnlilReader${SHARED_LIBRARY_SUFFIX} ${TMP_DIR}/vtkEnlilReader)

if (APPLE)
  execute_process(
      COMMAND ${PV_SUPERBUILD_LIST_DIR}/apple/fixup_plugin.py
            # The directory containing the plugin dylibs or the plugin itself.
            ${TMP_DIR}/vtkEnlilReader/${SHARED_LIBRARY_PREFIX}vtkEnlilReader${SHARED_LIBRARY_SUFFIX}
            # names to replace (in order)
            "${PARAVIEW_BINARY_DIR}/lib/=@executable_path/../Libraries/"
            #"/Users/jomu9721/Data/Development/SwFT/superbuild/paraview/src/paraview-build/lib/=@executable_path/../Libraries/"
            "${QT_LIBRARY_DIR}/Qt=@executable_path/../Frameworks/Qt"
            "${QT_LIBRARY_DIR}/=@executable_path/../Libraries/"
            "libhdf5.7.3.0.dylib=@executable_path/../Libraries/libhdf5.1.8.9.dylib"
            "libhdf5_hl.7.3.0.dylib=@executable_path/../Libraries/libhdf5.1.8.9.dylib"
            )
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E tar cvfz ${bundle_name} vtkEnlilReader
  WORKING_DIRECTORY ${TMP_DIR})
