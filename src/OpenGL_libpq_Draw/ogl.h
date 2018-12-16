#ifndef OGL_HPP_
#define OGL_HPP_

#include <GL/glew.h>

class ogl {
public:
  ogl(void);

  ~ogl();


private:
  GLuint loadBMP_custom(const char* imagepath, unsigned char* data);

};

#endif
