cmake_minimum_required(VERSION 3.10)
project(PipelineProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include header files
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add source files
add_executable(
    PipelineApp
    src/main.cpp
    src/pipeline.cpp
)