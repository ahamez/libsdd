cmake_minimum_required (VERSION 3.0)

#--------------------------------------------------------------------------------------------------#

project (libsdd)

#--------------------------------------------------------------------------------------------------#

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

#--------------------------------------------------------------------------------------------------#

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++1y" COMPILER_SUPPORTS_CXX14)
if (NOT COMPILER_SUPPORTS_CXX14)
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support.")
endif ()

#--------------------------------------------------------------------------------------------------#

find_package(Boost 1.59.0 REQUIRED COMPONENTS context coroutine system thread)
find_package(Doxygen QUIET)
find_package(Threads)

#--------------------------------------------------------------------------------------------------#

option(PACKED "Pack structures" OFF)
option(COVERAGE "Code coverage" OFF)
option(INTERNAL_DOC "Generate internal documentation" OFF)

#--------------------------------------------------------------------------------------------------#

if (PACKED)
  add_definitions("-DLIBSDD_PACKED")
endif ()

#--------------------------------------------------------------------------------------------------#

if (COVERAGE)
  include(CodeCoverage)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
  setup_target_for_coverage(coverage tests coverage)
endif ()

#--------------------------------------------------------------------------------------------------#

if (DOXYGEN_FOUND)
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
endif ()

#--------------------------------------------------------------------------------------------------#

set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++1y ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 ${CMAKE_CXX_FLAGS_DEBUG}")

#--------------------------------------------------------------------------------------------------#

include_directories(SYSTEM "${Boost_INCLUDE_DIRS}")
include_directories("${PROJECT_SOURCE_DIR}")

#--------------------------------------------------------------------------------------------------#

add_subdirectory(sdd)
add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)

#--------------------------------------------------------------------------------------------------#
