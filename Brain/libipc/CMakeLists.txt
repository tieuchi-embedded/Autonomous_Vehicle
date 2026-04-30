set(LIBIPC_SRC
    src/topic.c
    src/mq_transport.c
    src/shm.c
    src/bus.c
)

add_library(libipc STATIC ${LIBIPC_SRC})

target_include_directories(libipc PUBLIC
    ${CMAKE_SOURCE_DIR}                # for "messages/foo.h"
    include                            # for "ipc/topic.h", "ipc/bus.h"
    bindings/cpp/include               # for "ipc/bus.hpp"
)

target_link_libraries(libipc PUBLIC rt pthread)
target_compile_options(libipc PRIVATE -Wall -Wextra -Werror)

# Shared variant for Python ctypes bindings (output: liblibipc.so)
add_library(libipc_shared SHARED ${LIBIPC_SRC})
set_target_properties(libipc_shared PROPERTIES OUTPUT_NAME libipc)
target_include_directories(libipc_shared PUBLIC
    ${CMAKE_SOURCE_DIR}
    include
)
target_link_libraries(libipc_shared PUBLIC rt pthread)
target_compile_options(libipc_shared PRIVATE -Wall -Wextra -Werror -fPIC)
