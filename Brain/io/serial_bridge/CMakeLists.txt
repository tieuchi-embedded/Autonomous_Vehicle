add_library(serial_bridge STATIC
    src/protocol.cpp
    src/serial_reader.cpp
)

target_include_directories(serial_bridge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/messages
)

add_executable(serial_node app/serial_node.cpp)

target_link_libraries(serial_node PRIVATE
    serial_bridge
    libipc
)
