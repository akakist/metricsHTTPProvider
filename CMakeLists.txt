cmake_minimum_required(VERSION 3.19)

project (MetricsHTTPProvider)

if(UNIX AND NOT APPLE)
SET(LINUX true)
endif()

if(LINUX)
    set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/5.15.2/gcc_64)
    set(CMAKE_SHARED_LINKER_FLAGS "-Xlinker --export-dynamic -Xlinker -zdefs")
else()
    set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/5.15.2/clang_64)
endif()




include_directories( /usr/local/include)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC ")
link_directories( /usr/local/lib)

#set(PROJECT_NAME mp.exe CACHE STRING "Open C++ namespace to use")

set(USERVER_NAMESPACE_BEGIN "namespace userver {" CACHE STRING "Open C++ namespace to use")
set(USERVER_NAMESPACE_END "}" CACHE STRING "Close C++ namespace to use")

add_compile_definitions(
  "USERVER_NAMESPACE=userver"
  "USERVER_NAMESPACE_BEGIN=${USERVER_NAMESPACE_BEGIN}"
  "USERVER_NAMESPACE_END=${USERVER_NAMESPACE_END}"
)



ADD_SUBDIRECTORY(src)

    

