cmake_minimum_required(VERSION 3.15)

project(check_degen)

file(GLOB_RECURSE CPP_SRC_FILES ./*.cpp)

add_executable(check_degen ${CPP_SRC_FILES})

target_link_libraries(check_degen
	PUBLIC model
)

target_include_directories(check_degen
	PUBLIC model
)
