add_definitions("-DGTEST_HAS_TR1_TUPLE=0")
include_directories("${PROJECT_SOURCE_DIR}/tests/gtest/include")

# we need to compile googletest with the same flags
add_subdirectory(gtest)

set(SOURCES
    tests.cc
    dd/test_definition.cc
    dd/test_difference.cc
    dd/test_intersection.cc
    dd/test_paths.cc
    dd/test_sum.cc
    dd/test_top.cc
    hom/common_inductives.cc
    hom/test_hom_composition.cc
    hom/test_hom_cons.cc
    hom/test_hom_fixpoint.cc
    hom/test_hom_identity.cc
    hom/test_hom_inductive.cc
    hom/test_hom_local.cc
    hom/test_hom_saturation_fixpoint.cc
    hom/test_hom_saturation_sum.cc
    hom/test_hom_sum.cc
    hom/test_rewriting.cc
    internal/mem/test_cache.cc
    internal/mem/test_ptr.cc
    internal/mem/test_unique_table.cc
    internal/mem/test_variant_no_unroll.cc
    internal/mem/test_variant.cc
    order/test_utility.cc
    values/test_bitset.cc
    )

add_executable(tests ${SOURCES})
target_link_libraries(tests gtest ${TCMALLOC_LIBRARY})

if (VALGRIND_EXE)
  add_test( NAME "UnitTestsValgrind"
            COMMAND ${VALGRIND_EXE} $<TARGET_FILE:tests>
          )
else ()
  if (ADDRESS_SANITIZER)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -faddress-sanitizer")
    add_definitions("-faddress-sanitizer")
    add_test("UnitTestsAddressSanitizer" tests)
  else ()
    add_test("UnitTests" tests)
  endif ()  
endif ()
