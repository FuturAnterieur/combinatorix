include(MyFunctions)

set(CORE_GEOMETRY_SRCS 
  src/aabb.cpp
  src/collision_tree.cpp
)

add_library(geometry SHARED ${CORE_GEOMETRY_SRCS})

target_include_directories(geometry PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include)
target_include_directories(geometry PRIVATE include)

set(TEST_INCLUDES ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)

add_cxx_doctest_driver(TestBroadPhase SRCS tests/broad_phase.test.cpp INCLUDES ${TEST_INCLUDES} LINKS geometry TEST_FANCY_NAME broaderbund)