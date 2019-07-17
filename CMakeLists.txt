cmake_minimum_required(VERSION 3.10)

project(tdmspp-osem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN/../lib:$ORIGIN/")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tdmspp-osem SHARED src/log.cpp src/tdms_file.cpp src/tdms_segment.cpp)
add_executable(tdmsppinfo tests/tdmsppinfo.cpp)

target_compile_options(tdmspp-osem PRIVATE -Wall -O0 -g)
target_compile_options(tdmsppinfo PRIVATE -Wall -O0 -g)

target_include_directories(tdmspp-osem
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_link_libraries(tdmsppinfo tdmspp-osem)

target_include_directories(tdmsppinfo
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

install (TARGETS tdmsppinfo DESTINATION bin)
install (TARGETS tdmspp-osem DESTINATION lib)
install (FILES src/log.hpp src/tdms.hpp src/tdms_impl.hpp src/data_extraction.hpp DESTINATION include)
