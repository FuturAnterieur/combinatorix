
set(ENGINE_SRCS 
  src/engine.cpp
  src/game_logic.cpp
)

add_library(engine SHARED ${ENGINE_SRCS})
target_include_directories(engine PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include)
target_include_directories(engine PRIVATE ../)
target_include_directories(engine PRIVATE include)
target_link_libraries(engine PRIVATE logistics thread_pool)

add_executable(fake_engine main.cpp)
target_include_directories(fake_engine PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include)
target_include_directories(fake_engine PRIVATE ../)
target_include_directories(fake_engine PRIVATE include)
target_link_libraries(fake_engine PRIVATE logistics thread_pool engine)

