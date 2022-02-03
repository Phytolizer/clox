cmake_minimum_required(VERSION 3.12...3.23)

project(
  clox
  LANGUAGES C
  VERSION 0.1.0
  DESCRIPTION "A lox bytecode compiler/VM"
  HOMEPAGE_URL "https://github.com/Phytolizer/clox"
)

set(LOX_SRC "chunk.c")
list(TRANSFORM LOX_SRC PREPEND "src/")
add_library(lox ${LOX_SRC})
target_include_directories(lox PUBLIC include)

add_executable(clox clox.c)
target_link_libraries(clox lox)
