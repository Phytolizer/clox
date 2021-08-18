cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

include(cmake/CPM.cmake)

cpmaddpackage(
  NAME Tau VERSION 1.0.0 URL "https://github.com/jasmcaus/Tau/archive/dev.zip"
)

cpmaddpackage(
  NAME
  Format.cmake
  VERSION
  1.7.0
  GITHUB_REPOSITORY
  TheLartians/Format.cmake
  OPTIONS # set to yes skip cmake formatting
  "FORMAT_SKIP_CMAKE NO"
  # path to exclude (optional, supports regular expressions)
  "CMAKE_FORMAT_EXCLUDE cmake/CPM.cmake"
)

project(
  clox
  LANGUAGES C
  VERSION 0.1.0
  DESCRIPTION "CLox, from Crafting Interpreters"
  HOMEPAGE_URL "https://craftinginterpreters.com"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=nonnull")

option(DEBUG_TRACE_EXECUTION "Trace execution" ON)
option(DEBUG_PRINT_CODE "Print code compiled" ON)

add_subdirectory(apps)
add_subdirectory(source)
add_subdirectory(tests)
