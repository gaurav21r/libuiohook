#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "uiohook" for configuration ""
set_property(TARGET uiohook APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(uiohook PROPERTIES
  IMPORTED_IMPLIB_NOCONFIG "${_IMPORT_PREFIX}/lib/libuiohook.dll.a"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/libuiohook.dll"
  )

list(APPEND _cmake_import_check_targets uiohook )
list(APPEND _cmake_import_check_files_for_uiohook "${_IMPORT_PREFIX}/lib/libuiohook.dll.a" "${_IMPORT_PREFIX}/bin/libuiohook.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
