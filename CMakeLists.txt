cmake_minimum_required(VERSION 3.15)
project(concepts)

set(CMAKE_CXX_STANDARD 17)


set(entt_SOURCE_DIR "C:/Users/Pierre/Documents/Code\ Source/entt")

FILE(GLOB CONCEPTS_SRCS src/*.cpp)

add_executable(concepts ${CONCEPTS_SRCS})

target_include_directories(concepts PRIVATE ${entt_SOURCE_DIR}/src)
target_include_directories(concepts PRIVATE include)
