foreach(target pub_test sub_test shm_pub_test shm_sub_test)
    add_executable(cpp_${target} ${target}.cpp)
    target_include_directories(cpp_${target} PRIVATE
        ${CMAKE_SOURCE_DIR}/libipc/bindings/cpp/include
    )
    target_link_libraries(cpp_${target} PRIVATE libipc rt)
endforeach()
