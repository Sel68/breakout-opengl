cmake_minimum_required(VERSION 3.3)
project(OpenGLTest)

set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(glfw3 3.3 REQUIRED)
include_directories("${GLAD_DIR}/include")

add_executable(main main.cpp glad.c)
target_link_libraries(main PRIVATE glfw dl)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert ${CMAKE_CURRENT_SOURCE_DIR}/shader.frag DESTINATION ${CMAKE_CURRENT_BINARY_DIR})