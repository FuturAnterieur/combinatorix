cmake_minimum_required(VERSION 3.15)
project(concepts)

include(CTest)
enable_testing()
set(CMAKE_CXX_STANDARD 17)

set(EXTERNAL_LIBS_DIR "C:/Users/Pierre/Documents/Code\ Source")
set(ENTT_SOURCE_DIR "${EXTERNAL_LIBS_DIR}/entt")
set(DOCTEST_INCLUDE_DIR "${EXTERNAL_LIBS_DIR}/doctest/doctest")


add_subdirectory(logistics)
add_subdirectory(engine)