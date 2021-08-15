add_library(libclox chunk.c debug.c line.c memory.c value.c vm.c)
target_include_directories(libclox PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_compile_features(libclox PUBLIC c_std_11)
set_target_properties(libclox PROPERTIES OUTPUT_NAME clox)
