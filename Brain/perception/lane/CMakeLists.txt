find_package(OpenCV REQUIRED)

add_library(lane_core STATIC
    src/preprocessing.cpp
    src/lane_detector.cpp
    src/lane_tracker.cpp
)

target_include_directories(lane_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(lane_core PUBLIC ${OpenCV_LIBS})

add_executable(lane_node app/lane_node.cpp)

target_include_directories(lane_node PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(lane_node PRIVATE
    lane_core
    libipc
    ${OpenCV_LIBS}
)
