add_library(TwkMath STATIC Rand48.cpp Noise.cpp)

target_include_directories(TwkMath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TwkMath PUBLIC Imath::Imath)
