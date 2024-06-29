
set(ENGINE_SRCS 
  include/thread_pool.h
  src/thread_pool.cpp

)

add_library(engine SHARED ${ENGINE_SRCS})
target_include_directories(engine PRIVATE ${ENTT_SOURCE_DIR}/src)
target_include_directories(engine PRIVATE ../)
target_include_directories(engine PRIVATE include)
target_link_libraries(engine PRIVATE logistics)

add_executable(fake_engine main.cpp)
target_include_directories(fake_engine PRIVATE ${ENTT_SOURCE_DIR}/src)
target_include_directories(fake_engine PRIVATE ../)
target_include_directories(fake_engine PRIVATE include)
target_link_libraries(fake_engine PRIVATE logistics engine)