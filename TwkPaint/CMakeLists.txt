add_library(TwkPaint STATIC Path.cpp Smoother.cpp StampPath.cpp)

target_include_directories(TwkPaint PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TwkPaint PUBLIC TwkMath)
