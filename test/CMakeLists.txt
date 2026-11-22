enable_testing()

add_executable(
  ${PROJECT_NAME}_test
  test.cpp
)
target_link_libraries(
  ${PROJECT_NAME}_test
  GTest::gtest_main
  parlay
)

target_include_directories(${PROJECT_NAME}_test PUBLIC ${CMAKE_SOURCE_DIR}/include)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)
