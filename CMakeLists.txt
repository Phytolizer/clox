cmake_minimum_required(VERSION 3.12...3.21 FATAL_ERROR)

include(cmake/CPM.cmake)

cpmaddpackage(
  NAME Tau
  VERSION 1.0.0
  URL "https://github.com/jasmcaus/Tau/archive/dev.zip"
)

project(
  clox
  LANGUAGES C
  VERSION 0.1.0
  DESCRIPTION "CLox, from Crafting Interpreters"
  HOMEPAGE_URL "https://craftinginterpreters.com"
)

add_subdirectory(apps)
add_subdirectory(source)
add_subdirectory(tests)
