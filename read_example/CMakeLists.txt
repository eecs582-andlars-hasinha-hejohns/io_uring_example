cmake_minimum_required(VERSION 3.10)

project(test_read)

add_library(liburing STATIC IMPORTED)
set_target_properties(liburing PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/../install_instructions/liburing/liburing/src/liburing.a"
)

add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} liburing)