add_library(
  libclox
  chunk.c
  compiler.c
  debug.c
  line.c
  memory.c
  object.c
  scanner.c
  value.c
  vm.c
  table.c
)
target_include_directories(libclox PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(libclox PUBLIC c_std_11)
set_target_properties(libclox PROPERTIES OUTPUT_NAME clox)
if(DEBUG_TRACE_EXECUTION)
  message(STATUS "Execution tracing is enabled")
  target_compile_definitions(libclox PUBLIC DEBUG_TRACE_EXECUTION)
endif()
if(DEBUG_PRINT_CODE)
  message(STATUS "Printing compiled code is enabled")
  target_compile_definitions(libclox PUBLIC DEBUG_PRINT_CODE)
endif()
