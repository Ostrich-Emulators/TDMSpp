cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(LIBVERSION 1.0.0)
project(tdmspp-osem VERSION ${LIBVERSION} LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tdmspp-osem SHARED src/log.cpp src/tdms_file.cpp src/tdms_segment.cpp)
add_executable(tdmsppinfo tests/tdmsppinfo.cpp)

target_compile_options(tdmspp-osem PRIVATE -Wall)
target_compile_options(tdmsppinfo PRIVATE -Wall)

set(include_dest include/tdmspp-osem-${LIBVERSION})
set(main_lib_dest lib/tdmspp-osem-${LIBVERSION})
set(lib_dest lib/tdmspp-osem-${LIBVERSION}/${CMAKE_BUILD_TYPE})

target_include_directories(tdmspp-osem PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${include_dest}>
    $<INSTALL_INTERFACE:${lib_dest}>
)

target_link_libraries(tdmsppinfo tdmspp-osem)


SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../${lib_dest}/")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

install (TARGETS tdmsppinfo DESTINATION bin)
install (TARGETS tdmspp-osem EXPORT tdmspp-osem DESTINATION ${lib_dest})
install (FILES src/log.hpp src/tdms.hpp src/tdms_impl.hpp src/data_extraction.hpp DESTINATION ${include_dest})

install (FILES tdmspp-osem-config.cmake DESTINATION ${main_lib_dest})
install (EXPORT tdmspp-osem DESTINATION ${lib_dest})