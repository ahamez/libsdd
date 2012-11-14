################################################################################

cmake_minimum_required (VERSION 2.8.8)

################################################################################

project (libsdd)

################################################################################

find_package(Boost 1.50.0 REQUIRED)

################################################################################

option (TEST "Build and run tests." OFF) 
option (VALGRIND "Check memory with Valgrind." OFF)
option (ADDRESS_SANITIZER "Check memory with AddressSanitizer." OFF)
option (TCMALLOC "Use TCMalloc" OFF)
option (PACKED "Pack structures" OFF)
option (COVERAGE "Code coverage" OFF)

if (VALGRIND AND ADDRESS_SANITIZER)
  message(FATAL_ERROR "Can't use both valgrind and AddressSanitizer.")
endif ()

if (VALGRIND)
  find_program(VALGRIND_EXE valgrind)
  if (NOT VALGRIND_EXE)
    message(STATUS "valgrind not found.")
  endif ()
endif ()

if (ADDRESS_SANITIZER)
  # TODO: check if compiler is clang and that its version is >= 3.1
endif ()

if (TCMALLOC)
  find_library(TCMALLOC_LIBRARY tcmalloc)
  if (NOT TCMALLOC_LIBRARY)
    message(STATUS "TCMalloc not found.")
  else ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc")
  endif ()
endif ()

if (PACKED)
  add_definitions("-DLIBSDD_PACKED")
endif ()

if (COVERAGE)
  add_definitions("--coverage")
endif ()

################################################################################

add_definitions("-Wall")
add_definitions("-std=c++11")

if (CMAKE_COMPILER_IS_GNUCXX)
  if (COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
  endif ()
endif ()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_definitions("-stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=c++11 -stdlib=libc++")
  set(CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")
  if (COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
  endif ()
endif ()

set( CMAKE_CXX_FLAGS_RELEASE
     "${CMAKE_CXX_FLAGS_RELEASE} -fvisibility=hidden -fstrict-aliasing -DNDEBUG")

set( CMAKE_CXX_FLAGS_DEBUG
     "${CMAKE_CXX_FLAGS_DEBUG} -O0")

################################################################################

include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${PROJECT_SOURCE_DIR}")

################################################################################

add_subdirectory(examples)

################################################################################

if (TEST)
  enable_testing()
  add_subdirectory(tests)
endif ()

################################################################################
