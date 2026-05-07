add_executable(manual_node app/manual_node.cpp)

target_include_directories(manual_node PRIVATE
    ${CMAKE_SOURCE_DIR}/messages
)

target_link_libraries(manual_node PRIVATE libipc)
