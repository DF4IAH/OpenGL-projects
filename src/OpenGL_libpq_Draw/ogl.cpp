#include <stdio.h>
#include <stdlib.h>

#include <iostream>

// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;


/* old fashioned */
//#include <GL/glut.h>

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
  window = glfwCreateWindow(1024, 768, "Tutorial 03", nullptr, nullptr);
  if (window == nullptr) {
    fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(window); // Initialize GLEW
  glewExperimental = true; // Needed in core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return;
  }


  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // An array of 3 vectors which represents 3 vertices
  static const GLfloat g_vertex_buffer_data[] = {
     -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     0.0f,  1.0f, 0.0f,
  };

  // This will identify our vertex buffer
  GLuint vertexbuffer;

  // Generate 1 buffer, put the resulting identifier in vertexbuffer
  glGenBuffers(1, &vertexbuffer);

  // The following commands will talk about our 'vertexbuffer' buffer
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

  // Give our vertices to OpenGL.
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");
  cout << "programID = " << programID << endl;

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

#if 0
  {
    static glm::mat4 myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f));
    static glm::vec4 myVector(10.0f, 10.0f, 10.0f, 1.0f);

    // fill myMatrix and myVector somehow
    static glm::vec4 transformedVector = myMatrix * myVector; // Again, in this order ! this is important.

    cout << "myMatrix=" << endl
         << myMatrix[0][0] << "\t" << myMatrix[1][0] << "\t" << myMatrix[2][0] << "\t" << myMatrix[3][0] << endl
         << myMatrix[0][1] << "\t" << myMatrix[1][1] << "\t" << myMatrix[2][1] << "\t" << myMatrix[3][1] << endl
         << myMatrix[0][2] << "\t" << myMatrix[1][2] << "\t" << myMatrix[2][2] << "\t" << myMatrix[3][2] << endl
         << myMatrix[0][3] << "\t" << myMatrix[1][3] << "\t" << myMatrix[2][3] << "\t" << myMatrix[3][3] << endl;

    cout << "myVector=" << endl
         << myVector.x << endl
         << myVector.y << endl
         << myVector.z << endl
         << myVector.w << endl;

    cout << "transformedVector=" << endl
         << transformedVector.x << endl
         << transformedVector.y << endl
         << transformedVector.z << endl
         << transformedVector.w << endl;

    static glm::mat4 myScalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
    cout << "myScalingMatrix=" << endl
         << myScalingMatrix[0][0] << "\t" << myScalingMatrix[1][0] << "\t" << myScalingMatrix[2][0] << "\t" << myScalingMatrix[3][0] << endl
         << myScalingMatrix[0][1] << "\t" << myScalingMatrix[1][1] << "\t" << myScalingMatrix[2][1] << "\t" << myScalingMatrix[3][1] << endl
         << myScalingMatrix[0][2] << "\t" << myScalingMatrix[1][2] << "\t" << myScalingMatrix[2][2] << "\t" << myScalingMatrix[3][2] << endl
         << myScalingMatrix[0][3] << "\t" << myScalingMatrix[1][3] << "\t" << myScalingMatrix[2][3] << "\t" << myScalingMatrix[3][3] << endl;

  }
#endif

  do {
    // Clear the screen. It's not mentioned before Tutorial 02, but it can cause flickering, so it's there nonetheless.
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
      // 1st attribute buffer : vertices
      glEnableVertexAttribArray(0);

      glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

      glVertexAttribPointer(
         0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
         3,                  // size
         GL_FLOAT,           // type
         GL_FALSE,           // normalized?
         0,                  // stride
         nullptr             // array buffer offset
      );


      // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
      glm::mat4 Projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(width) / static_cast<float>(height),
        0.1f, 100.0f);

      // Or, for an ortho camera :
      //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

      // Camera matrix
      glm::mat4 View = glm::lookAt(
        glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 0), // and looks at the origin
        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

      // Model matrix : an identity matrix (model will be at the origin)
      glm::mat4 Model = glm::mat4(1.0f);
      // Our ModelViewProjection : multiplication of our 3 matrices
      glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

      // Use our shader
      glUseProgram(programID);

      // Get a handle for our "MVP" uniform
      // Only during the initialisation
      GLint MatrixID = glGetUniformLocation(programID, "MVP");

      // Send our transformation to the currently bound shader, in the "MVP" uniform
      // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
      glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

      // Draw the triangle !
      glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle

      glDisableVertexAttribArray(0);
    }

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);  // Check if the ESC key was pressed or the window was closed
}

ogl::~ogl()
{

  cout << "OpenGL DESC" << endl;
}
