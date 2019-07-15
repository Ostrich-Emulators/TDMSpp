project(TDMSpp-osem)
cmake_minimum_required(VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")


include_directories(src)
add_subdirectory(src)

add_subdirectory(tests)
