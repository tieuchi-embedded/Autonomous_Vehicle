add_library(planning_core STATIC
    src/behavior_manager.cpp
)

target_include_directories(planning_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/messages
)

add_executable(planning_node app/planning_node.cpp)

target_link_libraries(planning_node PRIVATE
    planning_core
    libipc
)
