add_library(libipc STATIC
    src/topic.c
    src/mq_transport.c
    src/shm.c
    src/bus.c
)

target_include_directories(libipc PUBLIC
    ${CMAKE_SOURCE_DIR}   # for "messages/foo.h"
    include               # for "ipc/topic.h", "ipc/bus.h"
)

target_link_libraries(libipc PUBLIC rt pthread)
target_compile_options(libipc PRIVATE -Wall -Wextra -Werror)
