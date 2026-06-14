find_package(OpenCV REQUIRED)

add_executable(camera_sim_node app/camera_sim_node.cpp)

target_include_directories(camera_sim_node PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(camera_sim_node PRIVATE
    libipc
    ${OpenCV_LIBS}
)
