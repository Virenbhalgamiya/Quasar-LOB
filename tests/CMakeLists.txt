include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(ullt_tests main.cpp matching_engine_test.cpp)
target_link_libraries(ullt_tests PRIVATE ullt_core Catch2::Catch2WithMain)
