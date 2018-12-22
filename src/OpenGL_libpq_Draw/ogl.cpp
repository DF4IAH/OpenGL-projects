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
  //glEnable(GL_CULL_FACE);


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
  UVmapName     = string("Mipmaps/World_Satview_2048x2048_DXT1.DDS");
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


void ogl::setupHeightMesh(const std::vector< std::vector< GLfloat > > heightVecVec, float scaleHeight, const glm::mat3 matUv)
{
  std::vector<glm::vec3> yMap;
  std::vector<glm::vec3> biMap;

  /* Sanity checks */
  if (!ready || heightVecVec.empty()) {
    cerr << "ERROR ogl: construction failed before, setupMesh() aborted." << endl;
    return;
  }

  const uint16_t yMapHeight = uint16_t(heightVecVec.size());
  const uint16_t yMapWidth  = uint16_t(heightVecVec.at(0).size());

  /* yMap */
  for (uint16_t yMapRowIdx = 0; yMapRowIdx < yMapHeight; ++yMapRowIdx) {
    for (uint16_t yMapColIdx = 0; yMapColIdx < yMapWidth; ++yMapColIdx) {
      const GLfloat yMapAlt = heightVecVec.at(yMapRowIdx).at(yMapColIdx);

      const GLfloat usrMapY = yMapAlt * scaleHeight;
      const GLfloat usrMapX = +(yMapColIdx / GLfloat(yMapWidth  - 1)) * 2.f - 1.f;
      const GLfloat usrMapZ = -(yMapRowIdx / GLfloat(yMapHeight - 1)) * 2.f + 1.f;

      const glm::vec3 thsVertex(usrMapX, usrMapY, usrMapZ);
      yMap.push_back(thsVertex);
    }
  }
  #if 0
  cout << "yMap:" << endl;
  printVecGlmVec3(yMap);
  cout << endl << flush;
  #endif

  /* biMap */
  for (uint16_t biMapRowIdx = 0; biMapRowIdx < yMapHeight - 1; ++biMapRowIdx) {
    for (uint16_t biMapColIdx = 0; biMapColIdx < yMapWidth - 1; ++biMapColIdx) {
      float biMapAlt = 0.0f;

      for (int variant = 0; variant < 4; ++variant) {
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

      const GLfloat biMapY = biMapAlt / 4.0f;
      const GLfloat biMapX = +((GLfloat(biMapColIdx) + 0.5f) / GLfloat(yMapWidth  - 1)) * 2.f - 1.f;
      const GLfloat biMapZ = -((GLfloat(biMapRowIdx) + 0.5f) / GLfloat(yMapHeight - 1)) * 2.f + 1.f;

      const glm::vec3 thsBiVec(biMapX, biMapY, biMapZ);
      biMap.push_back(thsBiVec);
    }
  }
  #if 0
  cout << "biMap:" << endl;
  printVecGlmVec3(biMap);
  cout << endl << flush;
  #endif

  /* For each yMap vertix */
  for (uint16_t yMapRowIdx = 1; yMapRowIdx < yMapHeight - 1; ++yMapRowIdx) {
    for (uint16_t yMapColIdx = 1; yMapColIdx < yMapWidth - 1; ++yMapColIdx) {
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
      #if 0
      cout << "vertices:" << endl;
      printVecGlmVec3(vertices);
      cout << endl << flush;
      #endif

      /* Mesh triangle UVs */
      {
        glm::vec3 midUVunscaled(midVertix.x, midVertix.z, 0.0f);
        glm::vec3  blUVunscaled( blVertix.x,  blVertix.z, 0.0f);
        glm::vec3  brUVunscaled( brVertix.x,  brVertix.z, 0.0f);
        glm::vec3  tlUVunscaled( tlVertix.x,  tlVertix.z, 0.0f);
        glm::vec3  trUVunscaled( trVertix.x,  trVertix.z, 0.0f);

        /* Transform UVs (scale, rotate and shift) */
        midUVunscaled = 0.5f + ((matUv * midUVunscaled) / 2.0f);
        blUVunscaled  = 0.5f + ((matUv *  blUVunscaled) / 2.0f);
        brUVunscaled  = 0.5f + ((matUv *  brUVunscaled) / 2.0f);
        tlUVunscaled  = 0.5f + ((matUv *  tlUVunscaled) / 2.0f);
        trUVunscaled  = 0.5f + ((matUv *  trUVunscaled) / 2.0f);

        const glm::vec2 midUV(midUVunscaled.x, midUVunscaled.y);
        const glm::vec2  blUV( blUVunscaled.x,  blUVunscaled.y);
        const glm::vec2  brUV( brUVunscaled.x,  brUVunscaled.y);
        const glm::vec2  tlUV( tlUVunscaled.x,  tlUVunscaled.y);
        const glm::vec2  trUV( trUVunscaled.x,  trUVunscaled.y);

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
      #if 0
      cout << "uvs:" << endl;
      printVecGlmVec2(uvs);
      cout << endl << flush;
      #endif

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

        #if 0
        vector<glm::vec3> test;
        cout << "Bottom" << endl;
        test.push_back(midVertix);
        test.push_back(blVertix);
        test.push_back(brVertix);
        printVecGlmVec3(test);

        cout << endl << "Delta(bottom), delta2 x delta1" << endl;
        test.clear();
        test.push_back(bDelta1);
        test.push_back(bDelta2);
        test.push_back(glm::cross(bDelta1, bDelta2));
        printVecGlmVec3(test);
        #endif

        const glm::vec3 bNorm = glm::normalize(glm::cross(bDelta1, bDelta2));
        const glm::vec3 lNorm = glm::normalize(glm::cross(lDelta1, lDelta2));
        const glm::vec3 rNorm = glm::normalize(glm::cross(rDelta1, rDelta2));
        const glm::vec3 tNorm = glm::normalize(glm::cross(tDelta1, tDelta2));

        #if 0
        if (bNorm.y < 0.0f) {
          cerr << "STOP" << endl << flush;
        }
        if (lNorm.y < 0.0f) {
          cerr << "STOP" << endl << flush;
        }
        if (rNorm.y < 0.0f) {
          cerr << "STOP" << endl << flush;
        }
        if (tNorm.y < 0.0f) {
          cerr << "STOP" << endl << flush;
        }
        #endif

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
      #if 0
      cout << "normals:" << endl;
      printVecGlmVec3(normals);
      cout << endl << flush;
      asm volatile( "nop" );
      #endif
    }
  }
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

  //glGenBuffers(1, &normalbuffer);
  //glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
  //glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(indexed_normals.size() * sizeof(glm::vec3)), &indexed_normals[0], GL_STATIC_DRAW);

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

#if 0
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
#endif

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
    //glDisableVertexAttribArray(2);

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
