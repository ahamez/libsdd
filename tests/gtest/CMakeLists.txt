# We first need to compile googletest
include_directories("${PROJECT_SOURCE_DIR}/tests/gtest")
add_library(gtest "${PROJECT_SOURCE_DIR}/tests/gtest/src/gtest-all.cc")
