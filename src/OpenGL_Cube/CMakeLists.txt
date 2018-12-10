include_directories(${CMAKE_SOURCE_DIR}/src ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

set(cubeSrcs main.c)
add_executable(OpenGL_Cube ${cubeSrcs})
target_link_libraries(OpenGL_Cube ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

install(TARGETS OpenGL_Cube DESTINATION bin)
