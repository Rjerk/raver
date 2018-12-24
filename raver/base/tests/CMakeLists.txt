add_executable(base64_test Base64Test.cc)
target_link_libraries(base64_test raver_base)

add_executable(threadpool_test ThreadPoolTest.cc)
target_link_libraries(threadpool_test raver_base)

add_executable(buffer_test BufferTest.cc)
target_link_libraries(buffer_test raver_base)

add_executable(filecache_test FileCacheTest.cc)
target_link_libraries(filecache_test raver_base)

add_executable(timer_test TimerTest.cc)
target_link_libraries(timer_test raver_base)

add_executable(timestamp_test TimeStampTest.cc)
target_link_libraries(timestamp_test raver_base)

add_executable(utils_test UtilsTest.cc)
target_link_libraries(utils_test raver_base)
