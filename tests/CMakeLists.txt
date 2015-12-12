include_directories("${PROJECT_SOURCE_DIR}/tests/gtest/include")

include_directories(SYSTEM "${PROJECT_SOURCE_DIR}/tests")
add_library(gtest "${PROJECT_SOURCE_DIR}/tests/gtest/gtest-all.cc")
set_target_properties(gtest PROPERTIES COMPILE_FLAGS "-w")

set(SOURCES
    tests.cc
    dd/test_count_combinations.cc
    dd/test_definition.cc
    dd/test_difference.cc
    dd/test_intersection.cc
    dd/test_path_generator.cc
    dd/test_sum.cc
    dd/test_top.cc
    hom/test_hom_composition.cc
    hom/test_hom_cons.cc
    hom/test_hom_fixpoint.cc
    hom/test_hom_function.cc
    hom/test_hom_identity.cc
    hom/test_hom_if_then_else.cc
    hom/test_hom_inductive.cc
    hom/test_hom_interrupt.cc
    hom/test_hom_local.cc
    hom/test_hom_saturation_fixpoint.cc
    hom/test_hom_saturation_sum.cc
    hom/test_hom_sum.cc
    hom/test_rewriting.cc
    mem/test_cache.cc
    mem/test_hash_table.cc
    mem/test_ptr.cc
    mem/test_unique_table.cc
    mem/test_variant.cc
    order/test_carrier.cc
    order/test_order.cc
    order/test_order_strategy.cc
    order/test_utility.cc
    tools/test_arcs.cc
    tools/test_nodes.cc
    util/test_next_power.cc
    util/test_typelist.cc
    values/test_bitset.cc
    values/test_flat_set.cc
    )

add_executable(tests ${SOURCES})
target_link_libraries(tests gtest ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_test("UnitTests" tests)
