set(http_SRCS
  Acceptor.cpp
  Channel.cpp
  EPoller.cpp
  HTTPConnection.cpp
  HTTPParser.cpp
  HTTPRequest.cpp
  HTTPResponse.cpp
  HTTPService.cpp
  IOManager.cpp
  ServiceManager.cpp
  )

add_library(raver_http ${http_SRCS})
target_link_libraries(raver_http raver_base)

add_subdirectory(tests)
