#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include <string.h>

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "ogl.h"
#include "pq.h"


typedef union u {
  int64_t   i64;
  int32_t   i32;
  int16_t   i16;
  int8_t    i8;

  double    fl;
  float     f;
} u_t;


int main(int argc, char* argv[])
{
  ogl ogl;
  pq  db;
  std::vector< std::vector< GLfloat > > heightVecVec;
  std::vector< GLfloat > heightRowVec;
  std::vector<double> timeVec;

  (void)argc;
  (void)argv;

  const double  latN =  50.0;
  const double  latS =  46.9;
  const double  lonW =   6.4;
  const double  lonE =   9.5;
  const GLfloat MagnifyHeight = 10.0f;

  timeVec.push_back(glfwGetTime());

  if (db.connect("gis")) {
    cout << "main: DB gis ok." << endl;
    char buf[256] = "";
    sprintf(buf,
            "SELECT height[topo_lon_grid(%lf) : topo_lon_grid(%lf)]  " \
            "FROM TOPO  " \
            "WHERE id = 1  AND  " \
            "row BETWEEN topo_lat_grid(%lf) AND topo_lat_grid(%lf);",
            lonW, lonE, latN, latS);
    vvUI64_t res = db.execSyncVVUI64(buf);
    const uint64_t rowCnt = uint64_t(db.getRowCnt());
    const uint64_t colCnt = uint64_t(db.getColCnt());
    timeVec.push_back(glfwGetTime());

    cout << "main: Got " << rowCnt << " rows with " << colCnt << " columns." << endl;

    std::vector< std::vector< uint64_t > >::iterator it = res.begin();
    for (uint64_t rowIdx = 0; rowIdx < rowCnt; ++rowIdx) {
      heightRowVec.clear();
      std::vector<uint64_t> row = *(it++);

      for (uint64_t colIdx = 0; colIdx < colCnt; ++colIdx) {
        uint64_t  len = row.at(std::vector<int64_t>::size_type(colIdx << 1));
        void*     ptr = reinterpret_cast<void*>(row.at(std::vector<int64_t>::size_type((colIdx << 1) + 1)));

        u_t  u = { 0 };
        uint8_t* u8ptrL = reinterpret_cast<uint8_t*>(&u);
        uint8_t* u8ptrR = reinterpret_cast<uint8_t*>(ptr);

        GLfloat f = 0.0f;

        if (len <= 8) {
          /* Form data */
          for (uint64_t idx = 0; idx < len; ++idx) {
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
            for (uint64_t idx = 0; idx < sizeof(int); ++idx) {
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
      heightVecVec.push_back(heightRowVec);
    }
    timeVec.push_back(glfwGetTime());

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

    const GLfloat heightScale = MagnifyHeight * GLfloat(2.0 / (fabs(latN - latS) * 60.0 * 1852.0));
    const GLfloat uvScale     = 1.0f;
    const GLfloat uvOfsX      = 0.0f;
    const GLfloat uvOfsY      = 0.0f;
    glm::mat3 matUv = glm::mat3(uvScale,  0.0f,     uvOfsX,
                                0.0f,     uvScale,  uvOfsY,
                                0.0f,     0.0f,     0.0f);
    ogl.setupHeightMesh(heightVecVec, heightScale, matUv);
    timeVec.push_back(glfwGetTime());

    ogl.doNormMean();
    timeVec.push_back(glfwGetTime());

    ogl.doIndex();
    timeVec.push_back(glfwGetTime());

    ogl.loadIntoVBO();
    timeVec.push_back(glfwGetTime());

    cout << "Timings" << endl;
    cout << "after\tDB execute, \t\tafter DB collect, \t\tafter mesh, \tafter mean normals, \tafter index, \tafter load VBO" << endl << "start";
    double startTime = timeVec.at(0);
    for (std::vector<double>::iterator it = timeVec.begin() + 1; it != timeVec.end(); ++it) {
      double diffTime = *it - startTime;
      double deltaTime = *it - *(it - 1);
      cout << ", \t(" << diffTime << ") " << deltaTime;
    }
    cout << endl << flush;

    ogl.enterLoop();
  }

  return 0;
}
