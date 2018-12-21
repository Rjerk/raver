set(base_SRCS
  Base64.cpp
  Buffer.cpp
  FileCache.cpp
  Logger.cpp
  RJson.cpp
  ThreadPool.cpp
  Timer.cpp
  TimeStamp.cpp
  Utils.cpp
)

add_library(raver_base ${base_SRCS})
target_link_libraries(raver_base pthread rt)

set_target_properties(raver_base PROPERTIES OUTPUT_NAME "raver_base")

add_subdirectory(tests)
