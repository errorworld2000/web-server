cmake_minimum_required(VERSION 3.14)

project(web-server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

include_directories(
  ${SRC_DIR}
  ${SRC_DIR}/include
  ${SRC_DIR}/http
  ${SRC_DIR}/sql
  ${SRC_DIR}/util
)

# Add source files
file(GLOB_RECURSE SOURCES
  "${SRC_DIR}/*.cc"
  "${SRC_DIR}/*.cpp"
)

# Create a static library from the sources
add_library(web-server-lib STATIC ${SOURCES})

# Add the executable
add_executable(web-server main.cc)


# Find necessary packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# Link libraries
target_link_libraries(web-server-lib PRIVATE Threads::Threads ${MYSQL_LIBRARIES})
target_link_libraries(web-server PRIVATE web-server-lib)


# Add include directories for MySQL
target_include_directories(web-server-lib PRIVATE ${MYSQL_INCLUDE_DIRS})

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Add tests subdirectory
add_subdirectory(tests)
