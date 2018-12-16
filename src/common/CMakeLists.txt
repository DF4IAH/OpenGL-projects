include_directories(${CMAKE_SOURCE_DIR}/src/common)

set(OpenGL_commonSrcs 
  controls.cpp
  objloader.cpp
  quaternion_utils.cpp
  shader.cpp
  tangentspace.cpp
  text2D.cpp
  texture.cpp
  vboindexer.cpp
)

add_library(OpenGL_common ${OpenGL_commonSrcs})
#target_link_libraries(OpenGL_common anyLibrary)

#install(TARGETS OpenGL_libpq_Draw DESTINATION bin)
