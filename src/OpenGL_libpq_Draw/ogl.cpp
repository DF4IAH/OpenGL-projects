// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <vector>
#include <iostream>

#include <omp.h>

// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include "../common/shader.hpp"
#include "../common/texture.hpp"
#include "../common/controls.hpp"
#include "../common/objloader.hpp"
#include "../common/vboindexer.hpp"

#include "ogl.h"

using namespace std;


typedef union u {
  int32_t   i32;

  float     f;
} u_t;


typedef struct MapFirst{
  glm::vec3 v;

  bool operator < (const MapFirst that) const {
    return memcmp(reinterpret_cast<const void*>(this), reinterpret_cast<const void*>(&that), sizeof(MapFirst)) > 0;
  }
} MapFirst_t;

typedef struct MapSec {
  glm::vec3   n;
  uint32_t    cnt;
} MapSec_t;



ogl::ogl(int widthPara, int heightPara)
{
  // Take over params when supplied
  width   = widthPara;
  height  = heightPara;

  ogl();
}

ogl::ogl()
{
  /* Initial timestamp */
  timeVec.push_back(glfwGetTime());

  /* Connect to the PostgreSQL DBMS */
  pq_open();
  if (!db_rdy) {
    return;
  }

  cout << "OpenGL CON" << endl;

  // Initialise GLFW
  glewExperimental = true;                                                                      // Needed for core profile
  if (!glfwInit())
  {
    cerr << "Failed to initialize GLFW\n" << endl;
    return;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);                                                              // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                                                // We want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);                                          // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);                                // We don't want the old OpenGL

  /* Open a window and create its OpenGL context */
  window = glfwCreateWindow(width, height, "OpenGL - PostgreSQL - GIS", nullptr, nullptr);
  if (window == nullptr) {
    fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window);

  /* Initialize GLEW */
  glewExperimental = true;                                                                      // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwTerminate();
    return;
  }


  /* Ensure we can capture the escape key being pressed below */
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  /* Hide the mouse and enable unlimited mouvement */
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  /* Set the mouse at the center of the screen */
  glfwPollEvents();
  glfwSetCursorPos(window, width/2, height/2);


  /* Dark blue background */
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  /* Enable depth test */
  glEnable(GL_DEPTH_TEST);
  /* Accept fragment if it closer to the camera than the former one */
  glDepthFunc(GL_LESS);

#if 0
  /* Cull triangles which normal is not towards the camera */
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
#endif

  /* VertexID */
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);


  /* Create and compile our GLSL program from the shaders */
  ProgramID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
  cout << "programID = " << ProgramID << endl;
  if (!ProgramID) {
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return;
  }

  /* Get uniform variables to connect to the shaders */
  MatrixID      = glGetUniformLocation(ProgramID, "MVP");                                       // Get a handle for our "MVP" uniform
  ViewMatrixID  = glGetUniformLocation(ProgramID, "V");                                         // Get a handle for our "V" uniform
  ModelMatrixID = glGetUniformLocation(ProgramID, "M");                                         // Get a handle for our "M" uniform
  LightID       = glGetUniformLocation(ProgramID, "LightPosition_worldspace");                  // Get a handle for our "LightPosition_worldspace" uniform

  /* Load the texture */
//UVmapName     = string("Mipmaps/uvmap_color.DDS");
//UVmapName     = string("Mipmaps/uvmap_dice.DDS");
  UVmapName     = string("Mipmaps/World_Satview_2048x2048_DXT1.DDS");   // TopLeft: 62.120752, -7.804211 / TopRight: 61.237249, 23.834532 / BotLeft: 32.025640, -7.638962 / BotRight: 31.097567, 22.378249  --> Delta: 30, 30
  Texture       = loadDDS(UVmapName.c_str());

  /* Get a handle for our "earthTextureSampler" uniform */
  TextureID     = glGetUniformLocation(ProgramID, "earthTextureSampler");

  timeVec.push_back(glfwGetTime());

  /* Construction done */
  gl_rdy = true;


  /**
    * Further use these methods
    *
    * setupMesh()
    * doIndex()
    * loadIntoVBO()
    * enterLoop()
    */

  /* Pre-load initial data set */
  const double latDelta = 10.0;
  pq_getAltData(49.5, 8.5, latDelta);
  timeVec.push_back(glfwGetTime());

  /* Build up altitude mesh */

  pq_transferDataDB2GL(10.0f, GLfloat(latDelta/30.0), GLfloat(latDelta/30.0), +0.035f, -0.083f);
  // 49.5 / 8.5 / 10.0 // 10.0 / 10.0/30.0 / 10.0/30.0 / +0.030 / -0.080
  // 49.5 / 8.5 /  4.0 // 10.0 /  4.0/30.0 /  4.0/30.0 / +0.035 / -0.083
  timeVec.push_back(glfwGetTime());

  doNormMean();
  timeVec.push_back(glfwGetTime());

  doIndex();
  timeVec.push_back(glfwGetTime());

  loadIntoVBO();
  timeVec.push_back(glfwGetTime());

  cout << "Timings" << endl;
  cout << "after\tafter GL init, \tafter DB, \t\t\tafter mesh, \tafter mean normals, \tafter index, \t\tafter load VBO" << endl << "start";
  double startTime = timeVec.at(0);
  for (std::vector<double>::iterator it = timeVec.begin() + 1; it != timeVec.end(); ++it) {
    double diffTime = *it - startTime;
    double deltaTime = *it - *(it - 1);
    cout << ", \t(" << diffTime << ") " << deltaTime;
  }
  cout << endl << flush;
  timeVec.clear();

  enterLoop();
}

ogl::~ogl()
{
  /* Drop flag */
  gl_rdy = false;

  /* Shutdown DB connection */
  pq_close();

  /* Cleanup VBO and shader */
  if (vertexbuffer) {
    glDeleteBuffers(1, &vertexbuffer);
  }

  if (uvbuffer) {
    glDeleteBuffers(1, &uvbuffer);
  }

  if (normalbuffer) {
    glDeleteBuffers(1, &normalbuffer);
  }

  if (ProgramID) {
    glDeleteProgram(ProgramID);
  }

  if (Texture) {
    glDeleteTextures(1, &Texture);
  }

  if (VertexArrayID) {
    glDeleteVertexArrays(1, &VertexArrayID);
  }

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  cout << "OpenGL DESC" << endl;
}


void ogl::setupAltMesh(GLfloat scaleAlt, const glm::mat3 matUv, GLfloat latDelta, GLfloat lonDelta)
{
  const GLfloat NormVecToCoordOfs = 0.001f;
  std::vector<glm::vec3> yMap;
  std::vector<glm::vec3> biMap;

  /* Sanity checks */
  if (!gl_rdy || db_altVecVec.empty()) {
    cerr << "ERROR ogl: construction failed before, setupMesh() aborted." << endl;
    return;
  }

  const uint32_t yMapHeight = uint32_t(db_altVecVec.size());
  const uint32_t yMapWidth  = uint32_t(db_altVecVec.at(0).size());

  /* yMap */
  for (uint32_t yMapRowIdx = 0; yMapRowIdx < yMapHeight; ++yMapRowIdx) {
    for (uint32_t yMapColIdx = 0; yMapColIdx < yMapWidth; ++yMapColIdx) {
      const double  radius  = 6371000.785;  // Mean value in meters
      const double  theta   = M_PI / 180.0 * fabs(((double(yMapRowIdx) / double(yMapHeight)) - 0.5) * double((latDelta) / 2.0f));
      const double  phi     = M_PI / 180.0 * fabs(((double(yMapColIdx) / double(yMapWidth))  - 0.5) * double((lonDelta) / 2.0f));

      const GLfloat yMapAlt = db_altVecVec.at(yMapRowIdx).at(yMapColIdx);
      const GLfloat usrMapY = GLfloat(double(scaleAlt) * (double(yMapAlt) - radius * (1.0 - (cos(theta) * cos(phi)))));
#if 0
      const GLfloat usrMapX = /* GLfloat(cos(phi)   * (*/ +(yMapColIdx / double(yMapWidth  - 1)) * double((2.0f - NormVecToCoordOfs) - 1.0f + NormVecToCoordOfs); //));
      const GLfloat usrMapZ = /* GLfloat(cos(theta) * (*/ +(yMapRowIdx / double(yMapHeight - 1)) * double((2.0f - NormVecToCoordOfs) - 1.0f + NormVecToCoordOfs); //));
#else
      const GLfloat usrMapX = +(yMapColIdx / GLfloat(yMapWidth  - 1)) * (2.0f - NormVecToCoordOfs) - 1.f + NormVecToCoordOfs;
      const GLfloat usrMapZ = +(yMapRowIdx / GLfloat(yMapHeight - 1)) * (2.0f - NormVecToCoordOfs) - 1.f + NormVecToCoordOfs;
#endif
      const glm::vec3 thsVertex(usrMapX, usrMapY, usrMapZ);

      yMap.push_back(thsVertex);
    }
  }

  /* biMap */
  for (uint32_t biMapRowIdx = 0; biMapRowIdx < yMapHeight - 1; ++biMapRowIdx) {
    for (uint32_t biMapColIdx = 0; biMapColIdx < yMapWidth - 1; ++biMapColIdx) {
      GLfloat biMapAlt = 0.0f;

      for (char variant = 0; variant < 4; ++variant) {
          switch (variant) {
          case 0:  // BOTTOM-LEFT
            biMapAlt += yMap.at(biMapRowIdx * yMapWidth + biMapColIdx).y;
            break;

          case 1:  // BOTTOM-RIGHT
            biMapAlt += yMap.at(biMapRowIdx * yMapWidth + biMapColIdx + 1).y;
            break;

          case 2:  // TOP-LEFT
            biMapAlt += yMap.at((biMapRowIdx + 1) * yMapWidth + biMapColIdx).y;
            break;

          case 3:  // TOP-RIGHT
          default:
            biMapAlt += yMap.at((biMapRowIdx + 1) * yMapWidth + biMapColIdx + 1).y;
            break;
          }
      }

      /* Bilinear altitude */
      const GLfloat biMapY = biMapAlt / 4.0f;
      const GLfloat biMapX = +((GLfloat(biMapColIdx) + 0.5f) / GLfloat(yMapWidth  - 1)) * (2.0f - NormVecToCoordOfs) - 1.f + NormVecToCoordOfs;
      const GLfloat biMapZ = +((GLfloat(biMapRowIdx) + 0.5f) / GLfloat(yMapHeight - 1)) * (2.0f - NormVecToCoordOfs) - 1.f + NormVecToCoordOfs;

      const glm::vec3 thsBiVec(biMapX, biMapY, biMapZ);
      biMap.push_back(thsBiVec);
    }
  }

  /* For each yMap vertix */
  for (uint32_t yMapRowIdx = 1; yMapRowIdx < yMapHeight - 1; ++yMapRowIdx) {
    for (uint32_t yMapColIdx = 1; yMapColIdx < yMapWidth - 1; ++yMapColIdx) {
      const glm::vec3 midVertix =  yMap.at( yMapRowIdx      *  yMapWidth      + yMapColIdx    );
      const glm::vec3 blVertix  = biMap.at((yMapRowIdx - 1) * (yMapWidth - 1) + yMapColIdx - 1);
      const glm::vec3 brVertix  = biMap.at((yMapRowIdx - 1) * (yMapWidth - 1) + yMapColIdx + 0);
      const glm::vec3 tlVertix  = biMap.at((yMapRowIdx + 0) * (yMapWidth - 1) + yMapColIdx - 1);
      const glm::vec3 trVertix  = biMap.at((yMapRowIdx + 0) * (yMapWidth - 1) + yMapColIdx + 0);

      /* Mesh triangle vertices */
      {
        /* Bottom triangle */
        vertices.push_back(midVertix);
        vertices.push_back(blVertix);
        vertices.push_back(brVertix);

        /* Left triangle */
        vertices.push_back(midVertix);
        vertices.push_back(tlVertix);
        vertices.push_back(blVertix);

        /* Right triangle */
        vertices.push_back(midVertix);
        vertices.push_back(brVertix);
        vertices.push_back(trVertix);

        /* Top triangle */
        vertices.push_back(midVertix);
        vertices.push_back(trVertix);
        vertices.push_back(tlVertix);
      }

      /* Mesh triangle UVs */
      {
        const glm::vec3 midUVunscaled(midVertix.x, midVertix.z, 0.0f);  // Offset to be added explicit
        const glm::vec3  blUVunscaled( blVertix.x,  blVertix.z, 0.0f);
        const glm::vec3  brUVunscaled( brVertix.x,  brVertix.z, 0.0f);
        const glm::vec3  tlUVunscaled( tlVertix.x,  tlVertix.z, 0.0f);
        const glm::vec3  trUVunscaled( trVertix.x,  trVertix.z, 0.0f);

        /* Transform UVs (scale, rotate and shift) */
        const glm::vec3 midUVscaled = (matUv * midUVunscaled) / (2.0f + NormVecToCoordOfs);
        const glm::vec3 blUVscaled  = (matUv *  blUVunscaled) / (2.0f + NormVecToCoordOfs);
        const glm::vec3 brUVscaled  = (matUv *  brUVunscaled) / (2.0f + NormVecToCoordOfs);
        const glm::vec3 tlUVscaled  = (matUv *  tlUVunscaled) / (2.0f + NormVecToCoordOfs);
        const glm::vec3 trUVscaled  = (matUv *  trUVunscaled) / (2.0f + NormVecToCoordOfs);

        const glm::vec2 midUV(midUVscaled.x + matUv[0][2] + 0.5f, midUVscaled.y + matUv[1][2] + 0.5f);
        const glm::vec2  blUV( blUVscaled.x + matUv[0][2] + 0.5f,  blUVscaled.y + matUv[1][2] + 0.5f);
        const glm::vec2  brUV( brUVscaled.x + matUv[0][2] + 0.5f,  brUVscaled.y + matUv[1][2] + 0.5f);
        const glm::vec2  tlUV( tlUVscaled.x + matUv[0][2] + 0.5f,  tlUVscaled.y + matUv[1][2] + 0.5f);
        const glm::vec2  trUV( trUVscaled.x + matUv[0][2] + 0.5f,  trUVscaled.y + matUv[1][2] + 0.5f);

        /* Bottom triangle */
        uvs.push_back(midUV);
        uvs.push_back( blUV);
        uvs.push_back( brUV);

        /* Left triangle */
        uvs.push_back(midUV);
        uvs.push_back( tlUV);
        uvs.push_back( blUV);

        /* Right triangle */
        uvs.push_back(midUV);
        uvs.push_back( brUV);
        uvs.push_back( trUV);

        /* Top triangle */
        uvs.push_back(midUV);
        uvs.push_back(trUV);
        uvs.push_back(tlUV);
      }

      /* Mesh triangle normals */
      {
        const glm::vec3 bDelta1 = blVertix - midVertix;
        const glm::vec3 bDelta2 = brVertix - midVertix;
        const glm::vec3 lDelta1 = tlVertix - midVertix;
        const glm::vec3 lDelta2 = blVertix - midVertix;
        const glm::vec3 rDelta1 = brVertix - midVertix;
        const glm::vec3 rDelta2 = trVertix - midVertix;
        const glm::vec3 tDelta1 = trVertix - midVertix;
        const glm::vec3 tDelta2 = tlVertix - midVertix;

        glm::vec3 bNorm = glm::normalize(glm::cross(bDelta2, bDelta1));
        glm::vec3 lNorm = glm::normalize(glm::cross(lDelta2, lDelta1));
        glm::vec3 rNorm = glm::normalize(glm::cross(rDelta2, rDelta1));
        glm::vec3 tNorm = glm::normalize(glm::cross(tDelta2, tDelta1));

# if 0
        if (bNorm.y < 0.0f) {
          bNorm = -bNorm;
        }
        if (lNorm.y < 0.0f) {
          lNorm = -lNorm;
        }
        if (rNorm.y < 0.0f) {
          rNorm = -rNorm;
        }
        if (tNorm.y < 0.0f) {
          tNorm = -tNorm;
        }
# endif

        /* Bottom triangle */
        normals.push_back(bNorm);
        normals.push_back(bNorm);
        normals.push_back(bNorm);

        /* Left triangle */
        normals.push_back(lNorm);
        normals.push_back(lNorm);
        normals.push_back(lNorm);

        /* Right triangle */
        normals.push_back(rNorm);
        normals.push_back(rNorm);
        normals.push_back(rNorm);

        /* Top triangle */
        normals.push_back(tNorm);
        normals.push_back(tNorm);
        normals.push_back(tNorm);
      }
    }
  }
}

void ogl::doNormMean(void)
{
  const uint32_t vecCnt = uint32_t(normals.size());
  std::map< MapFirst_t, MapSec_t > sumMap;

  /* Read in and sum up */
  for (uint32_t vecIdx = 0; vecIdx < vecCnt; ++vecIdx) {
    MapFirst_t mf;
    mf.v = vertices.at(vecIdx);
    std::map< MapFirst_t, MapSec_t >::iterator it = sumMap.find(mf);
    if (it != sumMap.end()) {
      it->second.n += normals.at(vecIdx);
      it->second.cnt++;

    } else {
      MapSec_t ms;
      ms.n   = normals.at(vecIdx);
      ms.cnt = 1;
      sumMap.insert(std::make_pair(mf, ms));
    }
  }

  /* Update all normal entries into shadow vector */
  std::vector< glm::vec3 > normalsShd;
  for (uint32_t idx = 0; idx < vecCnt; ++idx) {
    MapFirst_t mf;
    mf.v = vertices.at(idx);
    const std::map< MapFirst_t, MapSec_t >::iterator it = sumMap.find(mf);

    /* Normalize each summed up normal vectors */
    normalsShd.push_back(glm::normalize(it->second.n));
  }

  /* Copy shadow vector of normals back */
  normals = std::vector<glm::vec3>(normalsShd);
}

void ogl::doIndex(void)
{
  if (!gl_rdy) {
    cerr << "ERROR ogl: construction failed before, doIndex() aborted." << endl;
    return;
  }

  indexVBO(vertices, uvs, normals,
           indices, indexed_vertices, indexed_uvs, indexed_normals);
}

void ogl::loadIntoVBO(void)
{
  if (!gl_rdy) {
    cerr << "ERROR ogl: construction failed before, loadIntoVBO() aborted." << endl;
    return;
  }

  // Load it into a VBO
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(indexed_vertices.size() * sizeof(glm::vec3)), &indexed_vertices[0], GL_STATIC_DRAW);

  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(indexed_uvs.size() * sizeof(glm::vec2)), &indexed_uvs[0], GL_STATIC_DRAW);

  glGenBuffers(1, &normalbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(indexed_normals.size() * sizeof(glm::vec3)), &indexed_normals[0], GL_STATIC_DRAW);

  // Generate a buffer for the indices as well
  glGenBuffers(1, &elementbuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indices.size() * sizeof(unsigned int)), &indices[0] , GL_STATIC_DRAW);
}

void ogl::enterLoop(void)
{
  if (!gl_rdy) {
    cerr << "ERROR ogl: construction failed before, enterLoop() aborted." << endl;
    return;
  }

  // For speed computation
  lastTime = glfwGetTime();

  do {
#if 0
    // Measure speed
    double currentTime = glfwGetTime();
    nbFrames++;
    if (currentTime - lastTime >= 1.0) {
      // If last prinf() was more than 1 sec ago
      // printf and reset timer
      printf("%f ms/frame\n", 1000.0 / double(nbFrames));
      nbFrames = 0;
      lastTime += 1.0;
    }
#endif

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(ProgramID);

    // Compute the MVP matrix from keyboard and mouse input
    computeMatricesFromInputs();
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

    glm::vec3 lightPos = glm::vec3(4,4,4);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "earthTextureSampler" sampler to use Texture Unit 0
    glUniform1i(TextureID, 0);

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
      0,                                // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      nullptr                           // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
      1,                                // attribute
      2,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      nullptr                           // array buffer offset
    );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
      2,                                // attribute
      3,                                // size
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      nullptr                           // array buffer offset
    );

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

    // Draw the triangles !
    glDrawElements(
      GL_TRIANGLES,                     // mode
      GLsizei(indices.size()),          // count
      GL_UNSIGNED_INT,                  // type
      nullptr                           // array buffer offset
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();

  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);  // Check if the ESC key was pressed or the window was closed
}


void ogl::printVecGlmVec2(std::vector<glm::vec2> uv)
{
  int idx = 0;
  for (std::vector<glm::vec2>::const_iterator it = uv.begin(); it != uv.end(); ++it) {
    cout << "at " << idx++ << "\t" << it->x << "/" << it->y << endl;
  }
}

void ogl::printVecGlmVec3(const std::vector<glm::vec3> v)
{
  int idx = 0;
  for (std::vector<glm::vec3>::const_iterator it = v.begin(); it != v.end(); ++it) {
    cout << "at " << idx++ << "\t" << it->x << "/" << it->y << "/" << it->z << endl;
  }
}


void ogl::pq_open(void)
{
  db = new pq();
  if (!db) {
    return;
  }

  if (db->connect("gis")) {
    db_rdy = true;
    cout << "main: DB gis ok." << endl;
  }
}

void ogl::pq_close(void)
{
  if (db) {

      delete db;
      db = nullptr;
    }
}

void ogl::pq_getAltData(double latMid, double lonMid, double latDelta)
{
  std::vector< std::vector< GLfloat > > heightVecVec;
  std::vector< GLfloat >                heightRowVec;
  vvUI64_t                              res;

  /* Take over parameters */
  db_latMid   = latMid;
  db_lonMid   = lonMid;
  db_latDelta = latDelta;

  db_lonDelta = db_latDelta / sin(M_PI * db_latMid / 180.0);
  db_lonW     = db_lonMid - (db_lonDelta / 2.0);
  db_lonE     = db_lonMid + (db_lonDelta / 2.0);
  db_latN     = db_latMid + (db_latDelta / 2.0);
  db_latS     = db_latMid - (db_latDelta / 2.0);


  /* Read altitude data */
  {
    char buf[256] = "";
    db_altVecVec.clear();

    sprintf(buf,
            "SELECT height[topo_lon_grid(%lf) : topo_lon_grid(%lf)]  " \
            "FROM TOPO  " \
            "WHERE id = 1  AND  " \
            "row BETWEEN topo_lat_grid(%lf) AND topo_lat_grid(%lf);",
            db_lonW, db_lonE, db_latN, db_latS);

    /* Execute - remarks: do not forget to free the allocated space taken by this method */
    res = db->execSyncVVUI64(buf);

    db_rowCnt = uint32_t(db->getRowCnt());
    db_colCnt = uint32_t(db->getColCnt());
  }

  /* Parse data into GLfloat VecVec */
  {
    std::vector<std::vector<uint64_t> >::iterator it = res.begin();

    for (uint32_t rowIdx = 0; rowIdx < db_rowCnt; ++rowIdx) {
      std::vector< GLfloat > heightRowVec;
      std::vector<uint64_t> row = *(it++);

      for (uint32_t colIdx = 0; colIdx < db_colCnt; ++colIdx) {
        uint32_t  len = uint32_t(row.at(std::vector<uint64_t>::size_type(colIdx << 1)));
        void*     ptr = reinterpret_cast<void*>(row.at(std::vector<uint64_t>::size_type((colIdx << 1) + 1)));

        u_t  u = { 0 };
        uint8_t* u8ptrL = reinterpret_cast<uint8_t*>(&u);
        uint8_t* u8ptrR = reinterpret_cast<uint8_t*>(ptr);

        GLfloat f = 0.0f;

        if (len <= 4) {
          /* Form data */
          for (uint32_t idx = 0; idx < len; ++idx) {
            *(u8ptrL + idx) = *(u8ptrR + (len - idx - 1));
          }
          f = GLfloat(u.i32);

          /* Each column entry goes there */
          heightRowVec.push_back(f);

        } else {
          /* Array of int */
          int cnt = int(len);
          uint64_t ofs = 0;
          while (cnt >= int(sizeof(int))) {
            /* Form data */
            for (uint32_t idx = 0; idx < sizeof(int); ++idx) {
              *(u8ptrL + idx) = *(u8ptrR + (ofs + (sizeof(int) - idx - 1)));
            }
            f = GLfloat(u.i32);

            cnt -= sizeof(int);
            ofs += sizeof(int);

            if ((ofs >= 6 * sizeof(int)) && (ofs % 8)) {
              /* Each column entry goes there */
              heightRowVec.push_back(f);
            }
          }
        }

        /* Allocated by the class pq */
        free(ptr);
      }
      /* Each row vector goes there */
      db_altVecVec.push_back(heightRowVec);
    }
  }
}

void ogl::pq_transferDataDB2GL(GLfloat magAlt, GLfloat uvScaleX, GLfloat uvScaleY, GLfloat uvOfsX, GLfloat uvOfsY)
{
#if 0
  heightRowVec.clear();
  heightRowVec.push_back(0.1f); heightRowVec.push_back(0.2f); heightRowVec.push_back(0.3f); heightRowVec.push_back(0.2f); heightRowVec.push_back(0.1f);
  heightVecVec.push_back(heightRowVec);

  heightRowVec.clear();
  heightRowVec.push_back(0.3f); heightRowVec.push_back(0.4f); heightRowVec.push_back(0.4f); heightRowVec.push_back(0.5f); heightRowVec.push_back(0.2f);
  heightVecVec.push_back(heightRowVec);

  heightRowVec.clear();
  heightRowVec.push_back(0.4f); heightRowVec.push_back(0.5f); heightRowVec.push_back(0.6f); heightRowVec.push_back(0.3f); heightRowVec.push_back(0.2f);
  heightVecVec.push_back(heightRowVec);

  heightRowVec.clear();
  heightRowVec.push_back(0.5f); heightRowVec.push_back(0.3f); heightRowVec.push_back(0.4f); heightRowVec.push_back(0.2f); heightRowVec.push_back(0.1f);
  heightVecVec.push_back(heightRowVec);

  heightRowVec.clear();
  heightRowVec.push_back(0.2f); heightRowVec.push_back(0.4f); heightRowVec.push_back(0.1f); heightRowVec.push_back(0.1f); heightRowVec.push_back(0.0f);
  heightVecVec.push_back(heightRowVec);
#endif

  /* Take over parameters */
  db_MagAlt   = magAlt;
  db_UvScaleX = uvScaleX;
  db_UvScaleY = uvScaleY;
  db_UvOfsX   = uvOfsX;
  db_UvOfsY   = uvOfsY;

  const GLfloat latDelta = GLfloat(fabs(db_latN - db_latS));
  const GLfloat lonDelta = GLfloat(fabs(db_lonW - db_lonE));
  const GLfloat altScale = db_MagAlt * (2.0f / (latDelta * 60.0f * 1852.0f));

  glm::mat3 matUv = glm::mat3(db_UvScaleX,  0.0f,         db_UvOfsX,
                              0.0f,         db_UvScaleY,  db_UvOfsY,
                              0.0f,         0.0f,         0.0f);

  setupAltMesh(altScale, matUv, latDelta, lonDelta);
}
