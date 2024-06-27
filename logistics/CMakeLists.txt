
set(ENTT_SOURCE_DIR "${EXTERNAL_LIBS_DIR}/entt")
set(DOCTEST_INCLUDE_DIR "${EXTERNAL_LIBS_DIR}/doctest/doctest")

set(CORE_LOGISTICS_SRCS 
  src/combine.cpp
  src/effect.cpp
  src/split.cpp
  src/status.cpp
)

add_library(logistics SHARED ${CORE_LOGISTICS_SRCS})

target_include_directories(logistics PRIVATE ${ENTT_SOURCE_DIR}/src)
target_include_directories(logistics PRIVATE include)


add_executable(TestCombine ${SYSTEM_SRCS} tests/combine.test.cpp)
target_include_directories(TestCombine PRIVATE ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)
target_link_libraries(TestCombine PUBLIC logistics)

add_executable(TestStorageSanity ${SYSTEM_SRCS} tests/storage_sanity.test.cpp)
target_include_directories(TestStorageSanity PRIVATE ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)
target_link_libraries(TestStorageSanity PUBLIC logistics)
