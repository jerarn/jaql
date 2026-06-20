add_executable(bench_result bench_result.cpp)
target_link_libraries(bench_result PRIVATE jaql::core benchmark::benchmark)
jaql_apply_warnings(bench_result)
jaql_apply_sanitizers(bench_result)
