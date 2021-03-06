#ifndef OGL_HPP_
#define OGL_HPP_

#include <string>
#include <vector>
using namespace std;

#include <glm/glm.hpp>
#include <GL/glew.h>

#include "pq.h"


class ogl {
public:
  ogl();
  ogl(int widthPara, int heightPara);

  ~ogl();

  void printVecGlmVec2(std::vector<glm::vec2> v);
  void printVecGlmVec3(std::vector<glm::vec3> v);

private:
  void pq_open(void);
  void pq_close(void);
  void pq_getAltData(double latMid, double lonMid, double latDelta, double lonDelta);
  void pq_transferDataDB2GL(double latDelta, double lonDelta, double altScale, GLfloat uvScaleY, GLfloat uvScaleX, GLfloat uvOfsY, GLfloat uvOfsX);

  void setupAltMesh(double latDelta, double lonDelta, double altScale, const glm::mat3 matUv);
  void doNormMean(void);
  void doIndex(void);
  void loadIntoVBO(void);
  void enterLoop(void);


private:
  bool                          gl_rdy          = false;
  bool                          db_rdy          = false;


  /* postgreSQL DBMS */
  pq*                           db              = nullptr;

  double                        db_lonW         = 0.0;
  double                        db_lonE         = 0.0;
  double                        db_latN         = 0.0;
  double                        db_latS         = 0.0;

  uint32_t                      db_rowCnt       = 0UL;
  uint32_t                      db_colCnt       = 0UL;
  vector<vector<GLfloat> >      db_altVecVec;


  /* OpenGL */
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
  GLint                         LightColorID    = 0;
  GLint                         LightPowerID    = 0;

  GLuint                        Texture         = 0;
  GLint                         TextureID       = 0;

  /* VBO buffers */
  GLuint                        vertexbuffer    = 0;
  GLuint                        uvbuffer        = 0;
  GLuint                        normalbuffer    = 0;
  GLuint                        elementbuffer   = 0;

  std::vector<glm::vec3>        vertices;
  std::vector<glm::vec2>        uvs;
  std::vector<glm::vec3>        normals;

  std::vector<unsigned int>     indices;
  std::vector<glm::vec3>        indexed_vertices;
  std::vector<glm::vec2>        indexed_uvs;
  std::vector<glm::vec3>        indexed_normals;

  /* For speed computation */
  double                        lastTime        = 0.0;
  int                           nbFrames        = 0;
  std::vector<double>           timeVec;

};

#endif
