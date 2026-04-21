foreach(target pub_demo sub_demo shm_pub_demo shm_sub_demo)
    add_executable(${target} ${target}.c)
    target_link_libraries(${target} PRIVATE libipc rt)
endforeach()
