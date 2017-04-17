project(colout)
cmake_minimum_required(VERSION 2.8)

option(ENABLE_CXX_VERSION "enable -std=$version if clang or gcc." c++11)

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR CMAKE_COMPILER_IS_GNUCXX)
  include(CMakeDefinitions.txt)
  if (ENABLE_CXX_VERSION)
    add_definitions(-std=${ENABLE_CXX_VERSION})
  endif()
endif()

# if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -iquote ${PROJECT_SOURCE_DIR}/")
# endif()

# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

include_directories(src/)
include_directories(modules/falcon.cxx/include/)
include_directories(modules/variant/include)

# aux_source_directory(. SRC_LIST)
# file(GLOB headers_SRC "*.hpp")


# Tests
# function(add_executable_test name)
#   add_executable(${name}_test test/${name}_test.cpp)
#   add_test(all_tests ${name}_test)
# endfunction()

# add_executable_test(colout)

add_executable(
  colout
  colout.cpp
  src/colout/palette.cpp
  src/colout/cli/parse_colors.cpp
  src/colout/cli/parse_cli.cpp
)

# enable_testing()


# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/colout DESTINATION .)
