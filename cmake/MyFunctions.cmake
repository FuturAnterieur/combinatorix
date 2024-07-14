
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