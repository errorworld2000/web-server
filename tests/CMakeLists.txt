# Add the test executable
file(GLOB TEST_SOURCES
  "*.cc"
  "*.cpp"
)

add_executable(run_tests ${TEST_SOURCES})

# Link with Google Test
target_link_libraries(run_tests
  PRIVATE
  gtest
  gtest_main
  Threads::Threads
  web-server-lib
)

enable_testing()
add_test(NAME ALLTests COMMAND run_tests)