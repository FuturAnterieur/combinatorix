include(MyFunctions)

set(CORE_LOGISTICS_SRCS 
  src/attributes_info.cpp
  src/change_merger.cpp
  src/combine.cpp
  src/history_manager.cpp
  src/history_utils.cpp
  src/priority.cpp
  src/split.cpp
)

add_library(logistics SHARED ${CORE_LOGISTICS_SRCS})

target_include_directories(logistics PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include)
target_include_directories(logistics PRIVATE include)

set(TEST_INCLUDES ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include ${DOCTEST_INCLUDE_DIR} include)


add_cxx_doctest_driver(TestStorageSanity SRCS tests/storage_sanity.test.cpp INCLUDES ${TEST_INCLUDES} ${CEREAL_SOURCE_DIR}/include LINKS logistics TEST_FANCY_NAME storage_demo)
add_cxx_doctest_driver(TestAlgos SRCS tests/algos.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME Algos)
add_cxx_doctest_driver(TestPaco SRCS tests/PACO.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME pacocito)
add_cxx_doctest_driver(TestChanges SRCS tests/changes.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME panic_mode)