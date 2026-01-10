cmake_minimum_required(VERSION 3.3)
project(OpenGLTest)

set(GLAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

find_package(glfw3 3.3 REQUIRED)

include_directories("${GLAD_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(main 
    main.cpp 
    glad.c 
    shader.cpp 
    texture.cpp 
    resource_manager.cpp
    game.cpp
    ball_object.cpp
    game_object.cpp
    game_level.cpp
    sprite_renderer.cpp
    particle_generator.cpp
    post_processor.cpp
    power_up.cpp
    # text_renderer.cpp
)

target_link_libraries(main PRIVATE glfw dl)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/textures" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/levels" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")