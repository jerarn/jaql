add_executable(test_infra_logger test_logger.cpp)
target_link_libraries(test_infra_logger PRIVATE jaql::infra GTest::gtest_main)
gtest_discover_tests(test_infra_logger)
