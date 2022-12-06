QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += USERVER_NAMESPACE_BEGIN="'namespace userver {'"
DEFINES += USERVER_NAMESPACE_END="'}'"
DEFINES += USERVER_NAMESPACE="userver"
INCLUDEPATH += ../../../userver/core/include ../../../userver
INCLUDEPATH += ../../../userver/core/src \
               ../../../userver/shared/include \
                ../../../userver/shared/src \
                ../../../userver/third_party/fmt/include \
                ../../../userver/third_party/cctz/include \
                ../../../userver/third_party/moodycamel/include \
                /usr/local/include

#../../userver/core/libuserver-core.a /usr/local/lib/libboost_locale.dylib /usr/local/lib/libcurl.dylib -llber -lldap
#/usr/local/lib/libssh2.dylib ../../uboost_coro/libuserver-uboost-coro.a
#$/usr/local/lib/libboost_filesystem.dylib /usr/local/lib/libboost_program_options.dylib /usr/local/lib/libboost_iostreams.dylib
#/usr/local/lib/libboost_regex.dylib
#Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX12.3.sdk/usr/lib/libiconv.tbd
#/usr/local/lib/libssl.dylib /usr/local/lib/libcrypto.dylib
#/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX12.3.sdk/usr/lib/libz.tbd
#/usr/local/lib/libyaml-cpp.dylib /usr/local/lib/libfmt.dylib
#/usr/local/lib/libcares.dylib /usr/local/lib/libcctz.dylib
#../../third_party/boost_stacktrace/libuserver-stacktrace.a -ldl /usr/local/lib/libcryptopp.a
#/usr/local/lib/libhttp_parser.dylib /usr/local/lib/libev.dylib /usr/local/lib/libspdlog.dylib
LIBS+=-L/usr/local/lib \
        -luserver-core \
        -lboost_locale\
        -lcurl \
        -llber\
        -lldap \
        -lssh2\
        -luserver-uboost-coro \
        -lboost_filesystem \
        -lboost_program_options \
        -lboost_iostreams \
        -lboost_regex \
        -lcrypto \
        -lssl \
        -lz \
        -liconv \
        -lyaml-cpp \
        -lcctz \
        -luserver-stacktrace \
        -lcryptopp \
        -lhttp_parser \
        -lev \
        -lspdlog \
        -lfmt










KALL=        -luserver-core-internal \
-lboost_iostreams \
-lboost_program_options \
-lboost_context
-lyaml-cpp
-lcctz
-lfmt

#        /usr/local/lib/libuserver-core.a \
#/usr/local/lib/libuserver-core-internal.a \

SOURCES += \
        ../../src/MetricsHTTPProvider.cpp \
        ../../src/config_service.cpp \
        ../../src/daemon_run1.cpp \
        main.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../../src/MetricsHTTPProvider.hpp
