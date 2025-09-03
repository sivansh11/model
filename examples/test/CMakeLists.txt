cmake_minimum_required(VERSION 3.15)

project(test)

file(GLOB_RECURSE CPP_SRC_FILES ./*.cpp)

add_executable(test ${CPP_SRC_FILES})

target_link_libraries(test
	PUBLIC model
)

target_include_directories(test
	PUBLIC model
)
