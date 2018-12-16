// Include standard headers
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window;

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include "../common/shader.hpp"

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
  GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
  window = glfwCreateWindow(width, height, "Tutorial 05", nullptr, nullptr);
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

  #if 1
  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  glm::mat4 Projection = glm::perspective(
    glm::radians(45.0f),
    static_cast<float>(width) / static_cast<float>(height),
    0.1f, 100.0f);
  #else
  // Or, for an ortho camera :
  glm::mat4 Projection = glm::ortho(
        -10.0f, 10.0f,
        -10.0f, 10.0f,
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


  // Load the texture using any two methods
  unsigned char* textureData = nullptr;
  //GLuint Texture = loadBMP_custom("uvtemplate.bmp", textureData);
  GLuint Texture = loadDDS("uvtemplate.DDS");

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
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     0.0f,  1.0f, 0.0f
  #endif
  };

  // Two UV coordinates for each vertex. They were created with Blender.
  static const GLfloat g_uv_buffer_data[] = {
      0.000059f, 1.0f-0.000004f,
      0.000103f, 1.0f-0.336048f,
      0.335973f, 1.0f-0.335903f,
      1.000023f, 1.0f-0.000013f,
      0.667979f, 1.0f-0.335851f,
      0.999958f, 1.0f-0.336064f,
      0.667979f, 1.0f-0.335851f,
      0.336024f, 1.0f-0.671877f,
      0.667969f, 1.0f-0.671889f,
      1.000023f, 1.0f-0.000013f,
      0.668104f, 1.0f-0.000013f,
      0.667979f, 1.0f-0.335851f,
      0.000059f, 1.0f-0.000004f,
      0.335973f, 1.0f-0.335903f,
      0.336098f, 1.0f-0.000071f,
      0.667979f, 1.0f-0.335851f,
      0.335973f, 1.0f-0.335903f,
      0.336024f, 1.0f-0.671877f,
      1.000004f, 1.0f-0.671847f,
      0.999958f, 1.0f-0.336064f,
      0.667979f, 1.0f-0.335851f,
      0.668104f, 1.0f-0.000013f,
      0.335973f, 1.0f-0.335903f,
      0.667979f, 1.0f-0.335851f,
      0.335973f, 1.0f-0.335903f,
      0.668104f, 1.0f-0.000013f,
      0.336098f, 1.0f-0.000071f,
      0.000103f, 1.0f-0.336048f,
      0.000004f, 1.0f-0.671870f,
      0.336024f, 1.0f-0.671877f,
      0.000103f, 1.0f-0.336048f,
      0.336024f, 1.0f-0.671877f,
      0.335973f, 1.0f-0.335903f,
      0.667969f, 1.0f-0.671889f,
      1.000004f, 1.0f-0.671847f,
      0.667979f, 1.0f-0.335851f
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
  free(textureData);
  glDeleteVertexArrays(1, &VertexArrayID);

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
}

ogl::~ogl()
{

  cout << "OpenGL DESC" << endl;
}


GLuint ogl::loadBMP_custom(const char* imagepath, unsigned char* data)
{
  // Data read from the header of the BMP file
  unsigned char header[54]; // Each BMP file begins by a 54-bytes header
  int dataPos;     // Position in the file where the actual data begins
  int width, height;
  unsigned int imageSize;   // = width*height*3
  // Actual RGB data
  //unsigned char* data;

  // Open the file
  FILE* file = fopen(imagepath,"rb");
  if (!file) {
    printf("Image could not be opened\n");
    return 0;
  }

  if (fread(header, 1, 54, file) !=54) {
    // If not 54 bytes read : problem
    printf("Not a correct BMP file\n");
    return false;
  }

  if (header[0]!='B' || header[1]!='M') {
    printf("Not a correct BMP file\n");
    return 0;
  }

  // Read ints from the byte array
  dataPos    = *(reinterpret_cast<int*>(&(header[0x0A])));
  imageSize  = *(reinterpret_cast<unsigned int*>(&(header[0x22])));
  width      = *(reinterpret_cast<int*>(&(header[0x12])));
  height     = *(reinterpret_cast<int*>(&(header[0x16])));

  // Some BMP files are misformatted, guess missing information
  if (!imageSize)
    imageSize = static_cast<unsigned int>(width * height * 3); // 3 : one byte for each Red, Green and Blue component

  if (!dataPos)
    dataPos = 54; // The BMP header is done that way

  // Create a buffer
  data = new unsigned char[imageSize];

  // Read the actual data from the file into the buffer
  if (imageSize != fread(data, 1, imageSize, file)) {
    printf("Not a correct BMP file\n");
    return 0;
  }

  //Everything is in memory now, the file can be closed
  fclose(file);


  // Create one OpenGL texture
  GLuint textureID;
  glGenTextures(1, &textureID);

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, textureID);

  // Give the image to OpenGL
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  return textureID;
}
