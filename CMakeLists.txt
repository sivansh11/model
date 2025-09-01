cmake_minimum_required(VERSION 3.10)

project(model)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/OUTPUT/")
set(FETCHCONTENT_QUIET FALSE)

set(ASSIMP_INSTALL OFF CACHE INTERNAL "turns off assimp install")
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "assimp warnings as error")

include(FetchContent)

FetchContent_Declare(
  math
  GIT_REPOSITORY https://github.com/sivansh11/math
  GIT_TAG main
)
FetchContent_MakeAvailable(math)

FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(assimp)

file(GLOB_RECURSE SRC_FILES src/*cpp)

add_library(model ${SRC_FILES})

target_link_libraries(model
  PUBLIC math
  PUBLIC assimp
)

target_include_directories(model
  PUBLIC includes/
)

if (MODEL_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# default built type if CMAKE_BUILD_TYPE is not set
set(DEFAULT_BUILT_TYPE "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILT_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILT_TYPE}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
