
set(CORE_LOGISTICS_SRCS 
  src/attributes_info.cpp
  src/change_merger.cpp
  src/combine.cpp
  src/effect.cpp
  src/simulation_engine.cpp
  src/simulation_executable.cpp
  src/split.cpp
  src/status.cpp
)

add_library(logistics SHARED ${CORE_LOGISTICS_SRCS})

target_include_directories(logistics PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include)
target_include_directories(logistics PRIVATE include)

function(add_cxx_doctest_driver _target_name)
  set(options UNUSED)
  set(oneValueArgs TEST_FANCY_NAME)
  set(multiValueArgs SRCS LINKS INCLUDES)
  cmake_parse_arguments(
    PARSE_ARGV 1 LAGANN "${options}" "${oneValueArgs}" "${multiValueArgs}")
    add_executable(${_target_name} ${LAGANN_SRCS})
    target_include_directories(${_target_name} PRIVATE ${LAGANN_INCLUDES})
    target_link_libraries(${_target_name} PUBLIC ${LAGANN_LINKS})
    add_test(NAME ${LAGANN_TEST_FANCY_NAME} COMMAND $<TARGET_FILE:${_target_name}>)
endfunction()

set(TEST_INCLUDES ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include ${DOCTEST_INCLUDE_DIR} include)

add_cxx_doctest_driver(TestCombine SRCS tests/combine.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME awasete!!!)
add_cxx_doctest_driver(TestStorageSanity SRCS tests/storage_sanity.test.cpp INCLUDES ${TEST_INCLUDES} ${CEREAL_SOURCE_DIR}/include LINKS logistics TEST_FANCY_NAME storage_demo)
add_cxx_doctest_driver(TestAlgos SRCS tests/algos.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME Algos)
add_cxx_doctest_driver(TestStatus SRCS tests/status.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME basic_status)
add_cxx_doctest_driver(TestStatus2 SRCS tests/status_2.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME slightly_less_basic_status)
add_cxx_doctest_driver(TestStatusLoop SRCS tests/status_loop_1.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME DONOTRUN)
add_cxx_doctest_driver(TestPaco SRCS tests/PACO.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME pacocito)
add_cxx_doctest_driver(TestChanges SRCS tests/changes.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME panic_mode)