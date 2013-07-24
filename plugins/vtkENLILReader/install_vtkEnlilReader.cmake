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


#This is hacked for MacD108... not sure why it all works on other machines, but not this one...
if (APPLE)
  execute_process(
      COMMAND ${PV_SUPERBUILD_LIST_DIR}/apple/fixup_plugin.py
            # The directory containing the plugin dylibs or the plugin itself.
            ${TMP_DIR}/vtkEnlilReader/${SHARED_LIBRARY_PREFIX}vtkEnlilReader${SHARED_LIBRARY_SUFFIX}
            # names to replace (in order)
            "${PARAVIEW_BINARY_DIR}/lib/=@executable_path/../Libraries/"
            "/Volumes/Data/Development/SwFt/SuperBuild/paraview/src/paraview-build/lib/libavformat.dylib=@executable_path/../Libraries/libavformat.52.64.2.dylib"
            "/Volumes/Data/Development/SwFt/SuperBuild/paraview/src/paraview-build/lib/libavcodec.dylib=@executable_path/../Libraries/libavcodec.52.72.2.dylib"
            "/Volumes/Data/Development/SwFt/SuperBuild/paraview/src/paraview-build/lib/libavutil.dylib=@executable_path/../Libraries/libavutil.50.15.1.dylib"
            "/Volumes/Data/Development/SwFt/SuperBuild/paraview/src/paraview-build/lib/libswscale.dylib=@executable_path/../Libraries/libswscale.0.11.0.dylib"
            "/Volumes/Data/Development/SwFt/SuperBuild/paraview/src/paraview-build/lib/=@executable_path/../Libraries/"
            "${QT_LIBRARY_DIR}/Qt=@executable_path/../Frameworks/Qt"
            "${QT_LIBRARY_DIR}/=@executable_path/../Libraries/"
            "libhdf5.7.3.0.dylib=@executable_path/../Libraries/libhdf5.1.8.9.dylib"
            "libhdf5_hl.7.3.0.dylib=@executable_path/../Libraries/libhdf5.1.8.9.dylib"
            )
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E tar cvfz ${bundle_name} vtkEnlilReader
  WORKING_DIRECTORY ${TMP_DIR})
