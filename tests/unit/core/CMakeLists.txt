add_executable(test_core_error test_error.cpp)
target_link_libraries(test_core_error PRIVATE jaql::core GTest::gtest_main)
gtest_discover_tests(test_core_error)

add_executable(test_core_result test_result.cpp)
target_link_libraries(test_core_result PRIVATE jaql::core GTest::gtest_main)
gtest_discover_tests(test_core_result)

add_executable(test_core_strong_type test_strong_type.cpp)
target_link_libraries(test_core_strong_type PRIVATE jaql::core GTest::gtest_main)
gtest_discover_tests(test_core_strong_type)
