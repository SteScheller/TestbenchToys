add_library(
    vgamonitor 
    STATIC
        CVgaMonitor.cpp
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_impl_sdl.cpp
        imgui/imgui_impl_sdlrenderer.cpp
    )

target_include_directories(vgamonitor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(vgamonitor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

find_package(SDL2 2.0.17 REQUIRED)
target_include_directories(vgamonitor PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(vgamonitor PUBLIC ${SDL2_LIBRARIES})

