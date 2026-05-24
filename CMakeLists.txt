cmake_minimum_required(VERSION 3.10)
project(OpenGLTest)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Freetype REQUIRED)

set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(
    "${GLAD_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${OPENAL_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ball_object"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_level"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game_object"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/particle_generator"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/post_processor"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/power_up"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/resource_manager"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shader"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sprite_renderer"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/text_renderer"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/texture"
)

add_library(glad STATIC glad.c)

add_subdirectory(src/ball_object)
add_subdirectory(src/game)
add_subdirectory(src/game_level)
add_subdirectory(src/game_object)
add_subdirectory(src/particle_generator)
add_subdirectory(src/post_processor)
add_subdirectory(src/power_up)
add_subdirectory(src/resource_manager)
add_subdirectory(src/shader)
add_subdirectory(src/sprite_renderer)
add_subdirectory(src/text_renderer)
add_subdirectory(src/texture)
add_subdirectory(src/main)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/textures" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/levels" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/sounds" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/fonts" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")