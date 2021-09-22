cmake_minimum_required(VERSION 3.5)
project(PixelBot VERSION 0.0.1 LANGUAGES CXX)


add_executable(pxbot src/pixelbot.cpp)
set_target_properties(pxbot PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED YES
)
target_compile_features(pxbot PUBLIC cxx_std_20)