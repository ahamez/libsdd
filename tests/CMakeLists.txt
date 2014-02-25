include_directories("${PROJECT_SOURCE_DIR}/tests/gtest/include")

# we need to compile googletest with the same flags
# we should check for gtest directory presence...
include_directories("${PROJECT_SOURCE_DIR}/tests/gtest")
add_library(gtest "${PROJECT_SOURCE_DIR}/tests/gtest/src/gtest-all.cc")

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
    hom/test_hom_expression.cc
    hom/test_hom_fixpoint.cc
    hom/test_hom_identity.cc
    hom/test_hom_inductive.cc
    hom/test_hom_local.cc
    hom/test_hom_saturation_fixpoint.cc
    hom/test_hom_saturation_sum.cc
    hom/test_hom_simple_expression.cc
    hom/test_hom_sum.cc
    hom/test_hom_values_function.cc
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
    tools/test_nodes.cc
    util/test_next_power.cc
    util/test_typelist.cc
    values/test_bitset.cc
    values/test_flat_set.cc
    )

add_executable(tests ${SOURCES})
target_link_libraries(tests gtest ${TCMALLOC_LIBRARY} ${Boost_LIBRARIES})
add_test("UnitTests" tests)
