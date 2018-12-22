#include <iostream>
#include <string>
#include <vector>
using namespace std;

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include "ogl.h"
#include "pq.h"


int main(int argc, char* argv[])
{
  ogl ogl;
  pq  db;

  (void)argc;
  (void)argv;

  if (db.connect("gis")) {
    cout << "main: DB gis ok." << endl;
    vvs_t res = db.execSync("SELECT id  FROM TOPO  WHERE id = 1  AND  row between 5000 and 5000  LIMIT 100;");
    int rowCnt = db.getRowCnt();
    int colCnt = db.getColCnt();

    cout << "main: Got " << rowCnt << " rows with " << colCnt << " columns." << endl;

    std::vector< std::vector< GLfloat > > heightVecVec;
    std::vector< GLfloat > heightRowVec;

    heightRowVec.clear();
    heightRowVec.push_back(0.1f); heightRowVec.push_back(0.2f); heightRowVec.push_back(0.3f); heightRowVec.push_back(0.2f); heightRowVec.push_back(0.1f);
    heightVecVec.push_back(heightRowVec);

    heightRowVec.clear();
    heightRowVec.push_back(0.3f); heightRowVec.push_back(0.7f); heightRowVec.push_back(0.4f); heightRowVec.push_back(0.5f); heightRowVec.push_back(0.2f);
    heightVecVec.push_back(heightRowVec);

    heightRowVec.clear();
    heightRowVec.push_back(0.4f); heightRowVec.push_back(0.5f); heightRowVec.push_back(1.0f); heightRowVec.push_back(0.3f); heightRowVec.push_back(0.2f);
    heightVecVec.push_back(heightRowVec);

    heightRowVec.clear();
    heightRowVec.push_back(0.5f); heightRowVec.push_back(0.3f); heightRowVec.push_back(0.4f); heightRowVec.push_back(0.2f); heightRowVec.push_back(0.1f);
    heightVecVec.push_back(heightRowVec);

    heightRowVec.clear();
    heightRowVec.push_back(0.2f); heightRowVec.push_back(0.4f); heightRowVec.push_back(0.1f); heightRowVec.push_back(0.1f); heightRowVec.push_back(0.0f);
    heightVecVec.push_back(heightRowVec);

    const float heightScale = 1.0f;
    const float uvScale     = 1.0f;
    const float uvOfsX      = 0.0f;
    const float uvOfsY      = 0.0f;
    glm::mat3 matUv = glm::mat3(uvScale,  0.0f,     uvOfsX,
                                0.0f,     uvScale,  uvOfsY,
                                0.0f,     0.0f,     0.0f);
    ogl.setupHeightMesh(heightVecVec, heightScale, matUv);

    ogl.doIndex();
    ogl.loadIntoVBO();
    ogl.enterLoop();
  }

  return 0;
}
