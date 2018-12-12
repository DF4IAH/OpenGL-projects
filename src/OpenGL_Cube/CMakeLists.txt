if (WIN32)
   message(STATUS "This source code is using deprecated calls to the GL libaries and thus it will not being built.")
elseif (UNIX)
   #include_directories(${CMAKE_SOURCE_DIR}/src)

   set(OpenGL_CubeSrcs main.c)
   add_executable(OpenGL_Cube ${OpenGL_CubeSrcs})

   target_link_libraries(OpenGL_Cube ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

   install(TARGETS OpenGL_Cube DESTINATION bin)
endif()
