add_library(control_core STATIC
    src/lateral_controller.cpp
    src/speed_controller.cpp
)

target_include_directories(control_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/messages
)

add_executable(control_node app/control_node.cpp)

target_link_libraries(control_node PRIVATE
    control_core
    libipc
)
