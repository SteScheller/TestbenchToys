add_library(vgamonitor CVgaMonitor.cpp)
target_include_directories(vgamonitor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(SDL2 REQUIRED)
target_include_directories(vgamonitor PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(vgamonitor PUBLIC ${SDL2_LIBRARIES})

