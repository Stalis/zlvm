add_executable(zllink main.c)
target_include_directories(zllink PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include")
target_link_libraries(zllink PRIVATE zlvm-static)
zlvm_configure_target(zllink)
