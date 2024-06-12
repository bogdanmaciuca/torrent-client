set (CMAKE_CXX_STANDARD 17)

project(hello_cmake)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(client main.cpp)
target_link_libraries(client crypto)
target_link_libraries(client curl.dll)
