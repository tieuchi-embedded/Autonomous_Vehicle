add_library(serial STATIC
    src/protocol.c
    src/serial.c
)

target_include_directories(serial PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/messages
)

add_executable(serial_node app/serial_node.c)

target_link_libraries(serial_node PRIVATE
    serial
    libipc
)
