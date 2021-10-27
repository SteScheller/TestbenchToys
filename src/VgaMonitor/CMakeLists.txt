add_library(vgamonitor CVgaMonitor.cpp)
target_include_directories(vgamonitor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(SDL2 REQUIRED)
target_include_directories(vgamonitor PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(vgamonitor PRIVATE ${SDL2_LIBRARIES})

