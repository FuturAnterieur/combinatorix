
set(THREAD_POOL_SRCS 
  include/thread_pool.h
  src/thread_pool.cpp
)

add_library(thread_pool SHARED ${THREAD_POOL_SRCS})
target_include_directories(thread_pool PRIVATE ${ENTT_SOURCE_DIR}/src)
target_include_directories(thread_pool PRIVATE ../)
target_include_directories(thread_pool PRIVATE include)

