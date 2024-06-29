#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "geGL::geGL" for configuration ""
set_property(TARGET geGL::geGL APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(geGL::geGL PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libgeGL.a"
  )

list(APPEND _cmake_import_check_targets geGL::geGL )
list(APPEND _cmake_import_check_files_for_geGL::geGL "${_IMPORT_PREFIX}/lib/libgeGL.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
