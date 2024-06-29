
set(CORE_LOGISTICS_SRCS 
  src/combine.cpp
  src/effect.cpp
  src/split.cpp
  src/status.cpp
)

add_library(logistics SHARED ${CORE_LOGISTICS_SRCS})

target_include_directories(logistics PRIVATE ${ENTT_SOURCE_DIR}/src)
target_include_directories(logistics PRIVATE include)

add_executable(TestCombine tests/combine.test.cpp)
target_include_directories(TestCombine PRIVATE ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)
target_link_libraries(TestCombine PUBLIC logistics)
add_test(NAME awasete!!! COMMAND $<TARGET_FILE:TestCombine>)

add_executable(TestStorageSanity tests/storage_sanity.test.cpp)
target_include_directories(TestStorageSanity PRIVATE ${ENTT_SOURCE_DIR}/src ${CEREAL_SOURCE_DIR}/include ${DOCTEST_INCLUDE_DIR} include)
target_link_libraries(TestStorageSanity PUBLIC logistics)
add_test(NAME storage_demo COMMAND $<TARGET_FILE:TestStorageSanity>)

add_executable(TestStatus tests/status.test.cpp)
target_include_directories(TestStatus PRIVATE ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)
target_link_libraries(TestStatus PRIVATE logistics)
add_test(NAME status_seeker COMMAND $<TARGET_FILE:TestStatus>)

add_executable(Paco tests/PACO.test.cpp)
target_include_directories(Paco PRIVATE ${ENTT_SOURCE_DIR}/src ${DOCTEST_INCLUDE_DIR} include)
target_link_libraries(Paco PRIVATE logistics)
add_test(NAME entity_update_test COMMAND $<TARGET_FILE:Paco>)