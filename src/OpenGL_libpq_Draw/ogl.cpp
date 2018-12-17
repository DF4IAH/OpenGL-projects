// Include standard headers
#include <stdio.h>
#include <stdlib.h>

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
#include "../common/shader_own.hpp"
#include "../common/texture.hpp"
#include "../common/controls.hpp"

#include "ogl.h"

using namespace std;


const int width   = 1024;
const int height  =  768;


ogl::ogl(void)
{
  cout << "OpenGL CON" << endl;

  // Initialise GLFW
  glewExperimental = true; // Needed for core profile
  if (!glfwInit())
  {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    return;
  }

  glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

  // Open a window and create its OpenGL context
  window = glfwCreateWindow(width, height, "Tutorial 06", nullptr, nullptr);
  if (window == nullptr) {
    fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwTerminate();
    return;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);


  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);


  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");
  cout << "programID = " << programID << endl;
  if (!programID) {
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return;
  }


  // Get a handle for our "MVP" uniform
  GLint MatrixID = glGetUniformLocation(programID, "MVP");

#if 0
  #if 1
  // Projection matrix : 35° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  glm::mat4 Projection = glm::perspective(
    glm::radians(35.0f),
    static_cast<float>(width) / static_cast<float>(height),
    0.1f, 100.0f);
  #else
  // Or, for an ortho camera :
  glm::mat4 Projection = glm::ortho(
        -3.0f, 3.0f,
        -3.0f, 3.0f,
         0.0f, 100.0f); // In world coordinates
  #endif

  // Camera matrix
  glm::mat4 View = glm::lookAt(
    glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
    glm::vec3(0, 0, 0), // and looks at the origin
    glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

  // Model matrix : an identity matrix (model will be at the origin)
  glm::mat4 Model = glm::mat4(1.0f);
  // Our ModelViewProjection : multiplication of our 3 matrices
  glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
#endif


  // Load the texture using any two methods
#if 0
  GLuint Texture = loadBMP_custom("uvtemplate.bmp");
# define REVERT 1.0f-
#elif 0
  GLuint Texture = loadDDS("uvtemplate.DDS");
# define REVERT
#elif 1
  GLuint Texture = loadDDS("Mipmaps/World_Satview_2048x2048_DXT1.DDS");
# define REVERT
#elif 0
  GLuint Texture = loadDDS("Mipmaps/World_Satview_2048x2048_DXT3.DDS");
# define REVERT
#elif 0
  GLuint Texture = loadDDS("Mipmaps/World_Satview_2048x2048_DXT5.DDS");
# define REVERT
#elif 0
  GLuint Texture = loadDDS("Mipmaps/DL_Phy_1_512x512_DXT3.DDS");
# define REVERT
#endif
  cout << "Texture = " << Texture << endl;

  // Get a handle for our "myTextureSampler" uniform
  GLint TextureID  = glGetUniformLocation(programID, "myTextureSampler");


  static const GLfloat g_vertex_buffer_data[] = {
  #if 1
    // Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
    // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    -1.0f,-1.0f,-1.0f, // triangle 1 : begin
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, // triangle 1 : end
     1.0f, 1.0f,-1.0f, // triangle 2 : begin
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f, // triangle 2 : end
     1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
     1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f,-1.0f,
     1.0f,-1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
     1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
     1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f
  #else
    // An array of 3 vectors which represents 3 vertices
    -1.0f,-1.0f, 0.0f,
     1.0f,-1.0f, 0.0f,
     0.0f, 1.0f, 0.0f
  #endif
  };

  // Two UV coordinates for each vertex. They were created with Blender.
  static const GLfloat g_uv_buffer_data[] = {
    0.0001f, REVERT 0.0001f,
    0.0001f, REVERT 0.3333f,
    0.3334f, REVERT 0.3333f,
    0.9999f, REVERT 0.0001f,
    0.6667f, REVERT 0.3333f,
    0.9999f, REVERT 0.3333f,
    0.6667f, REVERT 0.3333f,
    0.3333f, REVERT 0.6667f,
    0.6667f, REVERT 0.6667f,
    0.9999f, REVERT 0.0001f,
    0.6667f, REVERT 0.0001f,
    0.6667f, REVERT 0.3333f,
    0.0001f, REVERT 0.0001f,
    0.3333f, REVERT 0.3333f,
    0.3333f, REVERT 0.0001f,
    0.6667f, REVERT 0.3333f,
    0.3333f, REVERT 0.3333f,
    0.3333f, REVERT 0.6667f,
    0.9999f, REVERT 0.6667f,
    0.9999f, REVERT 0.3333f,
    0.6667f, REVERT 0.3333f,
    0.6667f, REVERT 0.0001f,
    0.3333f, REVERT 0.3333f,
    0.6667f, REVERT 0.3333f,
    0.3333f, REVERT 0.3333f,
    0.6667f, REVERT 0.0001f,
    0.3333f, REVERT 0.0001f,
    0.0001f, REVERT 0.3333f,
    0.0001f, REVERT 0.6667f,
    0.3333f, REVERT 0.6667f,
    0.0001f, REVERT 0.3333f,
    0.3333f, REVERT 0.6667f,
    0.3333f, REVERT 0.3333f,
    0.6667f, REVERT 0.6667f,
    0.9999f, REVERT 0.6667f,
    0.6667f, REVERT 0.3333f
  };


  // Vertex buffer
  // This will identify our vertex buffer
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);


  do {
    // Clear the screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    // Compute the MVP matrix from keyboard and mouse input
    computeMatricesFromInputs();
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0);
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(TextureID, 0);

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
      0,                                // attribute 0. No particular reason for 0, but must match the layout in the shader.
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
      1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      2,                                // size : U+V => 2
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      nullptr                           // array buffer offset
    );


    #if 1
    // Draw the cube !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares
    #else
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    #endif


    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);


    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);  // Check if the ESC key was pressed or the window was closed


  // Cleanup VBO and shader
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
  glDeleteProgram(programID);
  glDeleteTextures(1, &Texture);
  glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
}

ogl::~ogl()
{

  cout << "OpenGL DESC" << endl;
}
