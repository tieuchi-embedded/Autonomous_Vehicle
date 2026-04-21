foreach(target pub_demo sub_demo shm_pub_demo shm_sub_demo)
    add_executable(cpp_${target} ${target}.cpp)
    target_include_directories(cpp_${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/libipc/bindings/cpp/include
    )
    target_link_libraries(cpp_${target} PRIVATE libipc rt)
endforeach()
