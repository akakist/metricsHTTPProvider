#cmake_minimum_required (VERSION 2.6)




add_executable(mp.exe 

daemon_run1.cpp

)
set(USERVER /Users/sergejbelalov/banner/userver)
include_directories(
${USERVER}/core/include 
${USERVER}
${USERVER}/core/src 
${USERVER}/shared/include 
${USERVER}/shared/src 
${USERVER}/third_party/fmt/include 
${USERVER}/third_party/cctz/include 
${USERVER}/third_party/moodycamel/include 
/usr/local/include
/usr/include/userver
/home/alexander_belyalov/userver/userver/core/src
/home/alexander_belyalov/userver/userver
/home/alexander_belyalov/userver/userver/shared/src
/home/alexander_belyalov/gears/src

                )
link_directories(/home/alexander_belyalov/gears/build/lib)

if(APPLE)
set(MACLINK "userver-stacktrace")
endif()
if(LINUX)
set(LINUXLINK "Generics")
endif()

target_link_libraries(mp.exe  MetricsHTTPProvider)


