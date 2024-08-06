
add_executable(graphics_example example.cpp glfw_glue.c)

set(GLFW_BUILD_EXAMPLES FALSE CACHE BOOL "Overwrite GLFW default value" FORCE)
set(GLFW_BUILD_TESTS FALSE CACHE BOOL "Overwrite GLFW default value" FORCE)
set(GLFW_BUILD_DOCS FALSE CACHE BOOL "Overwrite GLFW default value" FORCE)
set(GLFW_INSTALL FALSE CACHE BOOL "Overwrite GLFW default value" FORCE)
set(GLFW_BINARY_DIR ${GLFW_DIR}/build/src/Debug)
#add_subdirectory(${GLFW_DIR} ${GLFW_BINARY_DIR})

set(NATIVE_LIBRARIES opengl32.lib gdi32.lib shell32.lib)

target_include_directories(graphics_example PRIVATE ${SOKOL_DIR} ${GLFW_DIR}/include ${GLM_DIR})
target_include_directories(graphics_example PRIVATE ../)
target_include_directories(graphics_example PRIVATE include)
target_link_libraries(graphics_example PRIVATE ${GLFW_BINARY_DIR}/glfw3.lib)

