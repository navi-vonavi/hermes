project(legendre-2)

add_executable(${PROJECT_NAME} main.cpp)
include (../../examples/CMake.common)

set(BIN ${PROJECT_BINARY_DIR}/${PROJECT_NAME})
add_test(legendre-2 ${BIN})

target_link_libraries(${PROJECT_NAME} ${HERMES_BIN})
