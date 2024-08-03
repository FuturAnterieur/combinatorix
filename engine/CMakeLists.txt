
set(ENGINE_SRCS 
  src/action.cpp
  src/engine.cpp
  src/game_logic.cpp
  src/simulation_data.cpp
  src/simulation_executable.cpp
  src/snapshot_utils.cpp
)

add_library(engine SHARED ${ENGINE_SRCS})
target_include_directories(engine PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include)
target_include_directories(engine PRIVATE ../)
target_include_directories(engine PRIVATE include)
target_link_libraries(engine PRIVATE logistics thread_pool)

#add_executable(fake_engine main.cpp)
#target_include_directories(fake_engine PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include)
#target_include_directories(fake_engine PRIVATE ../)
#target_include_directories(fake_engine PRIVATE include)
#target_link_libraries(fake_engine PRIVATE logistics thread_pool engine)


set(TEST_INCLUDES ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include ${DOCTEST_INCLUDE_DIR} include ../)

add_cxx_doctest_driver(TestStatusWithEngine SRCS tests/status.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics engine TEST_FANCY_NAME basic_status_wih_engine)
add_cxx_doctest_driver(TestStatus2WithEngine SRCS tests/status_2.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics engine TEST_FANCY_NAME slightly_less_basic_status_wih_engine)
add_cxx_doctest_driver(TestStatus3 SRCS tests/status_3.test.cpp INCLUDES ${TEST_INCLUDES} LINKS logistics engine TEST_FANCY_NAME pre_change_triggers_wih_engine)
