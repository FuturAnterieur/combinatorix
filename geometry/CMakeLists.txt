include(MyFunctions)

set(CORE_GEOMETRY_SRCS 
  src/aabb.cpp
  src/collision_tree.cpp
  src/move_request.cpp
)

add_library(geometry SHARED ${CORE_GEOMETRY_SRCS})

target_include_directories(geometry PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include ${GLM_DIR})
target_include_directories(geometry PRIVATE include ../)

set(TEST_INCLUDES ${ENTT_SOURCE_DIR}/src ${GLM_DIR} ${DOCTEST_INCLUDE_DIR} include)

add_cxx_doctest_driver(TestBroadPhase SRCS tests/broad_phase.test.cpp INCLUDES ${TEST_INCLUDES} LINKS geometry TEST_FANCY_NAME broaderbund)