
set(ENGINE_SRCS 
  

)

add_library(engine_client SHARED ${ENGINE_SRCS})
target_include_directories(engine_client PRIVATE ${ENTT_SOURCE_DIR}/src)
target_include_directories(engine_client PRIVATE ../)
target_include_directories(engine_client PRIVATE include)
target_link_libraries(engine_client PRIVATE logistics thread_pool)
