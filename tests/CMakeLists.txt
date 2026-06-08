include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(twkpaint_tests
    test_path.cpp
    test_smoother.cpp
    test_stamppath.cpp
)

target_link_libraries(twkpaint_tests PRIVATE TwkPaint Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(twkpaint_tests)
