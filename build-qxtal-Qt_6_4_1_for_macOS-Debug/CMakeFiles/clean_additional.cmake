# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/qxtal_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/qxtal_autogen.dir/ParseCache.txt"
  "qxtal_autogen"
  )
endif()
