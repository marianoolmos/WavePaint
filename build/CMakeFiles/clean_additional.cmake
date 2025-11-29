# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/WavePaint_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/WavePaint_autogen.dir/ParseCache.txt"
  "WavePaint_autogen"
  )
endif()
