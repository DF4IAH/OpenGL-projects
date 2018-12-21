// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <iostream>

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


ogl::ogl(int widthPara, int heightPara)
{
  // Take over params when supplied
  width   = widthPara;
  height  = heightPara;

  ogl();
}

ogl::ogl()
{
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

  /* Cull triangles which normal is not towards the camera */
  glEnable(GL_CULL_FACE);


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
  Texture       = loadDDS(UVmapName.c_str());

  /* Get a handle for our "earthTextureSampler" uniform */
  TextureID     = glGetUniformLocation(ProgramID, "earthTextureSampler");

  /* Construction done */
  ready = true;

  /**
    * Further use these methods
    *
    * setupMesh()
    * doIndex()
    * loadIntoVBO()
    * enterLoop()
    */
}

ogl::~ogl()
{
  /* Drop flag */
  ready = false;

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


void ogl::setupHeightMesh(const std::vector< std::vector< int > > heightVecVec, float scaleHeight, GLfloat uvMulX, GLfloat uvAddX, GLfloat uvMulZ, GLfloat uvAddZ)
{
  std::vector<glm::vec3> yMap;

  if (!ready || heightVecVec.empty()) {
    cerr << "ERROR ogl: construction failed before, setupMesh() aborted." << endl;
    return;
  }

  /* User space */
  const uint16_t usrHeight  = uint16_t(heightVecVec.size());
  const uint16_t usrWidth   = uint16_t(heightVecVec.at(0).size());

  /* Target space */
  const uint16_t tgtRowCnt  = 2 * usrHeight;
  const uint16_t tgtColCnt  = 2 * usrWidth;

  /* Target vertix contains user heightVecVec and bi-linear intermediate verteces */

  /* Target map */
  for (uint16_t tgtRowIdx = 0; tgtRowIdx < tgtRowCnt - 1; tgtRowIdx += 2) {
    /* The user VecVec row */
    for (uint16_t tgtColIdx = 0; tgtColIdx < tgtColCnt - 1; tgtColIdx += 2) {
      const int usrHeight = heightVecVec.at(tgtRowIdx >> 1).at(tgtColIdx >> 1);

      const GLfloat tgtUsrY = usrHeight * scaleHeight;
      const GLfloat tgtUsrX = +(tgtColIdx / GLfloat(tgtColCnt - 2)) * 2.f - 1.f;
      const GLfloat tgtUsrZ = -(tgtRowIdx / GLfloat(tgtRowCnt - 2)) * 2.f + 1.f;

      const glm::vec3 thsVertex(tgtUsrX, tgtUsrY, tgtUsrZ);
      yMap.push_back(thsVertex);
    }

    if (tgtRowIdx >= tgtRowCnt - 2) {
      break;
    }

    /* The bi-linear row */
    for (uint16_t tgtColIdx = 0; tgtColIdx < tgtColCnt - 2; tgtColIdx += 2) {
      const GLfloat bilHeight = ( heightVecVec.at((tgtRowIdx >> 1) + 0).at((tgtColIdx >> 1) + 0) +
                                  heightVecVec.at((tgtRowIdx >> 1) + 0).at((tgtColIdx >> 1) + 1) +
                                  heightVecVec.at((tgtRowIdx >> 1) + 1).at((tgtColIdx >> 1) + 0) +
                                  heightVecVec.at((tgtRowIdx >> 1) + 1).at((tgtColIdx >> 1) + 1))
                                / 4.f;

      const GLfloat bilUsrY = bilHeight * scaleHeight;
      const GLfloat bilUsrX = +((tgtColIdx + 1) / GLfloat(tgtColCnt - 2)) * 2.f - 1.f;
      const GLfloat bilUsrZ = -((tgtRowIdx + 1) / GLfloat(tgtRowCnt - 2)) * 2.f + 1.f;

      const glm::vec3 thsVertex(bilUsrX, bilUsrY, bilUsrZ);
      yMap.push_back(thsVertex);
    }
  }

  /* Run again for all triangles */
  /* Target map */
  uint16_t midIdxY = 0;
  for (uint16_t tgtRowIdx = 0; tgtRowIdx < tgtRowCnt - 1; ++tgtRowIdx) {
    midIdxY = tgtRowIdx >> 1;

    for (uint16_t midIdxX = 0; midIdxX < usrWidth - (tgtRowIdx % 2); ++midIdxX) {  // Bi-linear rows are one element shorter
      cout << endl << "midIdxY=" << midIdxY << "\t midIdxX=" << midIdxX;

      glm::vec3 thsNormAdd(0, 0, 0);
      glm::vec3 thsNorm;

      const uint32_t midIdx = getIdxFromYMapCoord(midIdxX, midIdxY, usrWidth);
      const glm::vec3 midVertex = yMap.at(midIdx);
      glm::vec3 frame1Vertex;
      glm::vec3 frame2Vertex;

      if (0 == (tgtRowIdx % 2)) {
        /* The user VecVec row */
        cout << " of vertex row:" << endl << flush;

        for (uint16_t dy = 0; dy <= 1; ++dy) {
          for (uint16_t dx = 0; dx <= 1; ++dx) {
            /* Given vertices line */
            uint32_t idx1 = 255, idx2 = 255;

            if ((midIdxY >= 1) && (midIdxY < usrHeight - 1) &&
                (midIdxX >= 1) && (midIdxX < usrWidth  - 1)) {

              if (!dy && !dx) {
                /* Down triangle */
                idx1 = getIdxFromYMapCoord(uint16_t(midIdxX - 1), uint16_t(tgtRowIdx - 1), usrWidth);
                idx2 = getIdxFromYMapCoord(uint16_t(midIdxX    ), uint16_t(tgtRowIdx - 1), usrWidth);
                cout << " DN";

              } else if (!dy &&  dx) {
                 /* Left triangle */
                idx1 = getIdxFromYMapCoord(uint16_t(midIdxX - 1), uint16_t(tgtRowIdx - 1), usrWidth);
                idx2 = getIdxFromYMapCoord(uint16_t(midIdxX - 1), uint16_t(tgtRowIdx + 1), usrWidth);
                cout << " LE";

              } else if ( dy && !dx) {
                 /* Right triangle */
                idx1 = getIdxFromYMapCoord(uint16_t(midIdxX    ), uint16_t(tgtRowIdx - 1), usrWidth);
                idx2 = getIdxFromYMapCoord(uint16_t(midIdxX    ), uint16_t(tgtRowIdx + 1), usrWidth);
                cout << " RI";

              } else if ( dy &&  dx) {
                 /* Up triangle */
                idx1 = getIdxFromYMapCoord(uint16_t(midIdxX - 1), uint16_t(tgtRowIdx + 1), usrWidth);
                idx2 = getIdxFromYMapCoord(uint16_t(midIdxX    ), uint16_t(tgtRowIdx + 1), usrWidth);
                cout << " UP";
              }
              frame1Vertex  = yMap.at(idx1);
              frame2Vertex  = yMap.at(idx2);

              cout << ", dy=" << dy << " dx=" << dx << ": mid=" << midVertex.x << "/" << midVertex.z <<" (idx=" << midIdx << "), frame1=" << frame1Vertex.x << "/" << frame1Vertex.z << " (idx=" << idx1 << "), frame2=" << frame2Vertex.x << "/" << frame2Vertex.z << "(idx=" << idx2 << ")";

              /* Push triangle to vertices */
              {
                vertices.push_back(midVertex);
                vertices.push_back(frame1Vertex);
                vertices.push_back(frame2Vertex);
              }

              /* Push triangle to uvs */
              {
                const glm::vec2    midUv(   midVertex.x * uvMulX + uvAddX,    midVertex.z * uvMulZ + uvAddZ);
                const glm::vec2 frame1Uv(frame1Vertex.x * uvMulX + uvAddX, frame1Vertex.z * uvMulZ + uvAddZ);
                const glm::vec2 frame2Uv(frame2Vertex.x * uvMulX + uvAddX, frame2Vertex.z * uvMulZ + uvAddZ);

                uvs.push_back(midUv);
                uvs.push_back(frame1Uv);
                uvs.push_back(frame2Uv);
              }

              /* Bases of the triangle */
              const glm::vec3 delta1 = frame1Vertex - midVertex;
              const glm::vec3 delta2 = frame2Vertex - midVertex;

              thsNorm = glm::vec3(delta2) * glm::vec3(delta1);
              if (thsNorm.y < 0) {
                /* Make right handed */
                thsNorm = -thsNorm;
              }
              cout << "\t taken." << endl << flush;

            } else {
              thsNorm = vec3(0, 1, 0);  // Up
              cout << "\t dropped." << endl << flush;
            }

            /* Push triangle to normals */
            normals.push_back(thsNorm);
          }
        }

      } else {
        /* The bi-linear row */
        cout << " of bi-linear row:" << endl << flush;

        for (uint16_t dy = 0; dy <= 1; ++dy) {
          for (uint16_t dx = 0; dx <= 1; ++dx) {
            /* Given vertices line */
            uint32_t idx1 = 255, idx2 = 255;

            if ((midIdxY < usrHeight - 1) &&
                (midIdxX < usrWidth  - 1)) {

              if (!dy && !dx) {
                /* Down triangle */
                idx1 = getIdxFromYMapCoord(uint16_t(midIdxX    ), uint16_t(tgtRowIdx - 1), usrWidth);
                idx2 = getIdxFromYMapCoord(uint16_t(midIdxX + 1), uint16_t(tgtRowIdx - 1), usrWidth);
                cout << " DN";

              } else if (!dy &&  dx) {
                 /* Left triangle */
                idx1 = getIdxFromYMapCoord(uint16_t(midIdxX    ), uint16_t(tgtRowIdx - 1), usrWidth);
                idx2 = getIdxFromYMapCoord(uint16_t(midIdxX    ), uint16_t(tgtRowIdx + 1), usrWidth);
                cout << " LE";

              } else if ( dy && !dx) {
                 /* Right triangle */
                idx1 = getIdxFromYMapCoord(uint16_t(midIdxX + 1), uint16_t(tgtRowIdx - 1), usrWidth);
                idx2 = getIdxFromYMapCoord(uint16_t(midIdxX + 1), uint16_t(tgtRowIdx + 1), usrWidth);
                cout << " RI";

              } else if ( dy &&  dx) {
                 /* Up triangle */
                idx1 = getIdxFromYMapCoord(uint16_t(midIdxX    ), uint16_t(tgtRowIdx + 1), usrWidth);
                idx2 = getIdxFromYMapCoord(uint16_t(midIdxX + 1), uint16_t(tgtRowIdx + 1), usrWidth);
                cout << " UP";
              }
              frame1Vertex  = yMap.at(idx1);
              frame2Vertex  = yMap.at(idx2);

              cout << ", dy=" << dy << " dx=" << dx << ": mid=" << midVertex.x << "/" << midVertex.z <<" (idx=" << midIdx << "), frame1=" << frame1Vertex.x << "/" << frame1Vertex.z << " (idx=" << idx1 << "), frame2=" << frame2Vertex.x << "/" << frame2Vertex.z << "(idx=" << idx2 << ")";

              /* Push triangle to vertices */
              vertices.push_back(midVertex);
              vertices.push_back(frame1Vertex);
              vertices.push_back(frame2Vertex);

              /* Push triangle to uvs */
              {
                const glm::vec2    midUv(   midVertex.x * uvMulX + uvAddX,    midVertex.z * uvMulZ + uvAddZ);
                const glm::vec2 frame1Uv(frame1Vertex.x * uvMulX + uvAddX, frame1Vertex.z * uvMulZ + uvAddZ);
                const glm::vec2 frame2Uv(frame2Vertex.x * uvMulX + uvAddX, frame2Vertex.z * uvMulZ + uvAddZ);

                uvs.push_back(midUv);
                uvs.push_back(frame1Uv);
                uvs.push_back(frame2Uv);
              }

              /* Bases of the triangle */
              const glm::vec3 delta1 = frame1Vertex - midVertex;
              const glm::vec3 delta2 = frame2Vertex - midVertex;

              thsNorm = glm::vec3(delta2) * glm::vec3(delta1);
              if (thsNorm.y < 0) {
                /* Make right handed */
                thsNorm = -thsNorm;
              }
              cout << "\t taken." << endl << flush;

            } else {
              thsNorm = vec3(0, 1, 0);  // Up
              cout << "\t dropped." << endl << flush;
            }

            /* Push triangle to normals */
            normals.push_back(thsNorm);
          }
        }
      }
    }
  }

  cout << "vertices:" << endl;
  printVecGlmVec3(vertices);
  cout << endl;

  cout << "uvs:" << endl;
  printVecGlmVec2(uvs);
  cout << endl;

  cout << "normals:" << endl;
  printVecGlmVec3(normals);
  cout << endl;


#if 0
  // Read our .obj file

  bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);
  if (!res) {
    glDeleteProgram(ProgramID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return;
  }
#endif
}

void ogl::doIndex(void)
{
  if (!ready) {
    cerr << "ERROR ogl: construction failed before, doIndex() aborted." << endl;
    return;
  }

  indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
}

void ogl::loadIntoVBO(void)
{
  if (!ready) {
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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indices.size() * sizeof(unsigned short)), &indices[0] , GL_STATIC_DRAW);
}

void ogl::enterLoop(void)
{
  if (!ready) {
    cerr << "ERROR ogl: construction failed before, enterLoop() aborted." << endl;
    return;
  }

  // For speed computation
  lastTime = glfwGetTime();

  do {
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
      GL_UNSIGNED_SHORT,                // type
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


uint32_t ogl::getIdxFromYMapCoord(uint16_t posX, uint16_t posY, uint16_t colms)
{
  if (0 == (posY % 2)) {
    /* Given vertices line */
    return uint32_t((posY >> 1) * ((colms << 1) - 1) + posX);

  } else {
    /* Bi-linear line */
    return uint32_t((posY >> 1) * ((colms << 1) - 1) + colms + posX);
  }
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
