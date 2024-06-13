set (CMAKE_CXX_STANDARD 17)

project(torrent_client)
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(client main.cpp)
target_link_libraries(client crypto)
if(MINGW)
  target_link_libraries(client ws2_32)
endif()
