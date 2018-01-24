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
include_directories(SYSTEM modules/variant/include)

set(SRC src/colout/)

# aux_source_directory(. SRC_LIST)
# file(GLOB headers_SRC "*.hpp")

add_definitions(-frtti)
add_library(boost_unit_tests SHARED test/boost_unit_tests.cpp)
target_compile_definitions(boost_unit_tests PRIVATE -DBOOST_TEST_DYN_LINK)

# Tests
function(add_executable_test name)
  add_executable(${name}_test test/${name}_test.cpp)
  add_test(${name}_unit_test ${name}_test)
  target_link_libraries(${name}_test boost_unit_tests)
  target_compile_definitions(
    ${name}_test
    PRIVATE -DBOOST_AUTO_TEST_MAIN -DBOOST_TEST_DYN_LINK
  )
endfunction()

add_executable_test(parse_colors)
add_executable_test(color)
add_executable_test(palette)
remove_definitions(-frtti)

set(SRC_PARSE_COLOR ${SRC}/cli/parse_colors.cpp ${SRC}/palette.cpp)
add_library(parse_colors ${SRC_PARSE_COLOR})
target_link_libraries(parse_colors_test parse_colors)
target_link_libraries(palette_test parse_colors)


add_executable(colout colout.cpp src/colout/cli/parse_cli.cpp)
target_link_libraries(colout parse_colors)

# enable_testing()


# install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/colout DESTINATION .)
