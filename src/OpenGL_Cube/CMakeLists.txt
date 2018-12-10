#include_directories(${CMAKE_SOURCE_DIR}/src)

set(OpenGL_CubeSrcs main.c)
add_executable(OpenGL_Cube ${OpenGL_CubeSrcs})
target_link_libraries(OpenGL_Cube ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

install(TARGETS OpenGL_Cube DESTINATION bin)
