add_library(state_estimator STATIC
    src/state_estimator.cpp
)

target_include_directories(state_estimator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/messages
)

add_executable(state_node app/state_node.cpp)

target_link_libraries(state_node PRIVATE
    state_estimator
    libipc
)
