add_executable(hanoi hanoi.cc)
target_link_libraries(hanoi ${Boost_LIBRARIES})

if (ADDRESS_SANITIZER)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -faddress-sanitizer")
  add_definitions("-faddress-sanitizer")
endif ()
