#include_directories(${CMAKE_SOURCE_DIR}/src)

set(OpenGL_libpq_DrawSrcs main.cpp ogl.cpp pq.cpp)
add_executable(OpenGL_libpq_Draw ${OpenGL_libpq_DrawSrcs})
target_link_libraries(OpenGL_libpq_Draw ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ${PostgreSQL_LIBRARIES})

install(TARGETS OpenGL_libpq_Draw DESTINATION bin)
