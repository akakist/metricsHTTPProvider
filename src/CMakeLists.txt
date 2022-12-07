#cmake_minimum_required (VERSION 2.6)




add_executable(mp.exe 

MetricsHTTPProvider.cpp
config_service.cpp
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

                )

if(APPLE)
set(MACLINK "userver-stacktrace")
endif()

target_link_libraries(mp.exe  
        userver-core 
        boost_locale
        curl 
        lber
        ldap 
        userver-uboost-coro 
        boost_filesystem 
        boost_program_options 
        boost_iostreams 
        boost_regex 
        crypto 
        ssl 
        z 
        yaml-cpp 
        cctz 
        cryptopp 
        http_parser 
        ev 
        spdlog 
        fmt
        ${MACLINK}
        

z  pthread dl)


