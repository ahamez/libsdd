add_executable(hanoi hanoi.cc)
target_link_libraries(hanoi ${Boost_LIBRARIES})

add_executable(dictionary dictionary.cc)
target_link_libraries(dictionary ${Boost_LIBRARIES})
