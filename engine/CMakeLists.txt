
set(ENGINE_SRCS 
  include/engine.h
  include/engine_p.h
  src/engine.cpp
)

add_library(engine ${ENGINE_SRCS})
target_include_directories(engine PRIVATE ${ENTT_SOURCE_DIR}/src)
target_include_directories(engine PRIVATE ../)
target_include_directories(engine PRIVATE include)
target_link_libraries(engine PRIVATE logistics)
