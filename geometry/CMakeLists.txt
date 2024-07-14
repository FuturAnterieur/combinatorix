set(CORE_GEOMETRY_SRCS 
  src/aabb.cpp
  src/collision_tree.cpp
)


add_library(geometry SHARED ${CORE_GEOMETRY_SRCS})

target_include_directories(geometry PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include)
target_include_directories(geometry PRIVATE include)