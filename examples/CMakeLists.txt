add_executable(hanoi hanoi.cc)

if (ADDRESS_SANITIZER)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -faddress-sanitizer")
  add_definitions("-faddress-sanitizer")
endif ()
