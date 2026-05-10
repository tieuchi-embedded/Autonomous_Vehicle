foreach(target pub_test sub_test shm_pub_test shm_sub_test)
    add_executable(${target} ${target}.c)
    target_link_libraries(${target} PRIVATE libipc rt)
endforeach()
