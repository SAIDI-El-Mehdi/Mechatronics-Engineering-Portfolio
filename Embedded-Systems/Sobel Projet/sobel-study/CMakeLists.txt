cmake_minimum_required(VERSION 3.16)
project(sobel_study LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED)
find_package(OpenMP)

include_directories(include ${OpenCV_INCLUDE_DIRS})

add_executable(sobel_baseline
    apps/main_baseline.cpp
    src/io.cpp
    src/sobel_manual.cpp
    src/sobel_opencv.cpp
    src/utils.cpp
)

target_link_libraries(sobel_baseline PRIVATE ${OpenCV_LIBS})
target_compile_options(sobel_baseline PRIVATE -O3 -march=native -DNDEBUG)

add_executable(sobel_optimized
    apps/main_optimized.cpp
    src/io.cpp
    src/sobel_manual_opt.cpp
    src/sobel_opencv_opt.cpp
    src/utils.cpp
)

target_link_libraries(sobel_optimized PRIVATE ${OpenCV_LIBS})

if(OpenMP_CXX_FOUND)
    target_link_libraries(sobel_optimized PRIVATE OpenMP::OpenMP_CXX)
endif()

target_compile_options(sobel_optimized PRIVATE -O3 -march=native -DNDEBUG)
