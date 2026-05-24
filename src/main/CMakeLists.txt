add_executable(main main.cpp)
target_link_libraries(main PRIVATE 
    game ball_object game_level game_object 
    particle_generator post_processor power_up 
    resource_manager shader sprite_renderer 
    text_renderer texture 
    game ball_object game_level game_object 
    particle_generator post_processor power_up 
    resource_manager shader sprite_renderer 
    text_renderer texture 
    glad glfw ${OPENAL_LIBRARY} Freetype::Freetype dl
)
