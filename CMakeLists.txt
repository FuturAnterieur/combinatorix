cmake_minimum_required(VERSION 3.27)
project(concepts)

include(CTest)
enable_testing()
set(CMAKE_CXX_STANDARD 17)

set(EXTERNAL_LIBS_DIR "C:/Users/Pierre/Documents/Code\ Source")
set(ENTT_SOURCE_DIR "${EXTERNAL_LIBS_DIR}/entt")
set(CEREAL_SOURCE_DIR "${EXTERNAL_LIBS_DIR}/cereal")
set(DOCTEST_INCLUDE_DIR "${EXTERNAL_LIBS_DIR}/doctest/doctest")
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(logistics)
add_subdirectory(geometry)
add_subdirectory(engine)
add_subdirectory(thread_pool)