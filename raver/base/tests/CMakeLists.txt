add_executable(base64_test Base64Test.cpp)
target_link_libraries(base64_test raver_base)

add_executable(threadpool_test ThreadPoolTest.cpp)
target_link_libraries(threadpool_test raver_base)

add_executable(buffer_test BufferTest.cpp)
target_link_libraries(buffer_test raver_base)

add_executable(filecache_test FileCacheTest.cpp)
target_link_libraries(filecache_test raver_base)

add_executable(timer_test TimerTest.cpp)
target_link_libraries(timer_test raver_base)

add_executable(timestamp_test TimeStampTest.cpp)
target_link_libraries(timestamp_test raver_base)

add_executable(utils_test UtilsTest.cpp)
target_link_libraries(utils_test raver_base)
