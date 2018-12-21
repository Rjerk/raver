add_subdirectory(base)
add_subdirectory(http)

add_executable(raver main.cpp)
target_link_libraries(raver raver_base)
target_link_libraries(raver raver_http)
