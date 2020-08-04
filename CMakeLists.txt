cmake_minimum_required(VERSION 3.10)

set(LIBVERSION 2.1.0)
project(tdmspp-osem VERSION ${LIBVERSION} LANGUAGES CXX)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/../lib/tdmspp-osem-${LIBVERSION}:$ORIGIN/")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tdmspp-osem SHARED 
  src/data_type.cpp
  src/data_extraction.cpp
  src/datachunk.cpp
  src/log.cpp
  src/tdms_channel.cpp
  src/tdms_file.cpp
  src/tdms_segment.cpp)
add_executable(tdmsppinfo tests/tdmsppinfo.cpp)

if ( MSVC )
    target_compile_options(tdmspp-osem PRIVATE /W4)
    target_compile_options(tdmsppinfo PRIVATE /W4)
else()
    set(CMAKE_CXX_COMPILER /usr/bin/g++)
    target_compile_options(tdmspp-osem PRIVATE -Wall)
    target_compile_options(tdmsppinfo PRIVATE -Wall)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
endif()

set(include_dest include/tdmspp-osem-${LIBVERSION})
set(lib_dest lib/tdmspp-osem-${LIBVERSION})

target_include_directories(tdmspp-osem
    PUBLIC
        $<INSTALL_INTERFACE:${include_dest}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(tdmsppinfo tdmspp-osem)

target_include_directories(tdmsppinfo
    PUBLIC
        $<INSTALL_INTERFACE:${include_dest}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

configure_file(tdmspp-osem-config-version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/tdmspp-osem-config-version.cmake @ONLY)

install (TARGETS tdmsppinfo DESTINATION bin)
install (TARGETS tdmspp-osem EXPORT tdmspp-osem DESTINATION ${lib_dest})
install (FILES 
  src/data_type.h
  src/tdms_listener.h
  src/datachunk.h
  src/tdms_exports.h
  src/log.hpp
  src/tdmspp.h
  src/tdms_channel.h
  src/tdms_exceptions.h
  src/tdms_file.hpp
  src/tdms_segment.hpp
  src/data_extraction.hpp
DESTINATION ${include_dest})
install (EXPORT tdmspp-osem DESTINATION ${lib_dest})

install (FILES tdmspp-osem-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/tdmspp-osem-config-version.cmake DESTINATION ${lib_dest})
#install (EXPORT tdmspp-osem-config DESTINATION .)