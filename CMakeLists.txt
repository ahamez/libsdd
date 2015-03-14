cmake_minimum_required (VERSION 2.8.10)

################################################################################

project (libsdd)

################################################################################

find_package(Boost 1.55.0 REQUIRED COMPONENTS context coroutine system thread)
find_package(Doxygen QUIET)

################################################################################

option (PACKED "Pack structures" OFF)
option (COVERAGE "Code coverage" OFF)
option (INTERNAL_DOC "Generate internal documentation" OFF)

if (PACKED)
  add_definitions("-DLIBSDD_PACKED")
endif ()

if (COVERAGE)
  add_definitions("--coverage")
endif ()

################################################################################

if(DOXYGEN_FOUND)
  if (INTERNAL_DOC)
    set(GEN_INTERNAL_DOC "YES")
  else ()
    set(GEN_INTERNAL_DOC "NO")
  endif ()
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doc
                    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Generating API documentation with Doxygen" VERBATIM
                    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
                   )
endif(DOXYGEN_FOUND)

################################################################################

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX14)
if(NOT COMPILER_SUPPORTS_CXX14)
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support.")
endif ()

################################################################################

add_definitions("-Wall")
add_definitions("-std=c++1y")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
  endif ()
endif ()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
  endif ()
  if (COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
  endif ()
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

################################################################################

include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${PROJECT_SOURCE_DIR}")

################################################################################

enable_testing()
add_subdirectory(tests)

################################################################################
