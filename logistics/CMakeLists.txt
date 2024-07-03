
set(CORE_LOGISTICS_SRCS 
  src/attributes_info.cpp
  src/combine.cpp
  src/effect.cpp
  src/local_change_tracker.cpp
  src/simulation_engine.cpp
  src/split.cpp
  src/status.cpp
)

add_library(logistics SHARED ${CORE_LOGISTICS_SRCS})

target_include_directories(logistics PRIVATE ${ENTT_SOURCE_DIR}/src)
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

set(TEST_INCLUDES ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)

add_cxx_doctest_driver(TestCombine SRCS tests/combine.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics TEST_FANCY_NAME awasete!!!)
add_cxx_doctest_driver(TestStorageSanity SRCS tests/storage_sanity.test.cpp INCLUDES ${TEST_INCLUDES} ${CEREAL_SOURCE_DIR}/include LINKS logistics TEST_FANCY_NAME storage_demo)

add_executable(TestStatus tests/status.test.cpp)
target_include_directories(TestStatus PRIVATE ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)
target_link_libraries(TestStatus PRIVATE logistics)
add_test(NAME status_seeker COMMAND $<TARGET_FILE:TestStatus>)

add_executable(Paco tests/PACO.test.cpp)
target_include_directories(Paco PRIVATE ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)
target_link_libraries(Paco PRIVATE logistics)
add_test(NAME entity_update_test COMMAND $<TARGET_FILE:Paco>)