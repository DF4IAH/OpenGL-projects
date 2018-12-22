#ifndef OGL_HPP_
#define OGL_HPP_

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <GL/glew.h>


class ogl {
public:
  ogl();
  ogl(int widthPara, int heightPara);

  ~ogl();

  void setupHeightMesh(const std::vector< std::vector< GLfloat > > heightVecVec, float scaleHeight, const glm::mat3 matUv);
  void doIndex(void);
  void loadIntoVBO(void);
  void enterLoop(void);

  void printVecGlmVec2(std::vector<glm::vec2> v);
  void printVecGlmVec3(std::vector<glm::vec3> v);


private:
  const int                     widthDflt       = 1024;
  const int                     heightDflt      =  768;

  int                           width           = widthDflt;
  int                           height          = heightDflt;

  std::string                   UVmapName       = std::string("Mipmaps/uvmap_metering.DDS");
  GLuint                        VertexArrayID   = 0;
  GLuint                        ProgramID       = 0;
  GLint                         MatrixID        = 0;
  GLint                         ViewMatrixID    = 0;
  GLint                         ModelMatrixID   = 0;
  GLint                         LightID         = 0;

  GLuint                        Texture         = 0;
  GLint                         TextureID       = 0;

  std::vector<glm::vec3>        vertices;
  std::vector<glm::vec2>        uvs;
  std::vector<glm::vec3>        normals;

  std::vector<unsigned short>   indices;
  std::vector<glm::vec3>        indexed_vertices;
  std::vector<glm::vec2>        indexed_uvs;
  std::vector<glm::vec3>        indexed_normals;

  /* VBO buffers */
  GLuint                        vertexbuffer    = 0;
  GLuint                        uvbuffer        = 0;
  GLuint                        normalbuffer    = 0;
  GLuint                        elementbuffer   = 0;

  /* For speed computation */
  double                        lastTime        = 0.0;
  int                           nbFrames        = 0;

  bool                          ready           = false;
};

#endif
