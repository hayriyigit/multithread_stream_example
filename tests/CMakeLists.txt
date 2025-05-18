include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.16.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(pipeline_test
  pipeline_test.cpp)

target_link_libraries(pipeline_test
 PRIVATE
  gtest
  gmock
  pipeline
  pthread)

add_test(pipeline_gtests pipeline_test)