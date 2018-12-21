#include <iostream>
#include <string>
#include <vector>

#include "ogl.h"
#include "pq.h"

using namespace std;


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

    std::vector< std::vector< int > > heightVecVec;
    std::vector< int > heightRowVec;

    heightRowVec.clear();
    heightRowVec.push_back(1); heightRowVec.push_back(2); heightRowVec.push_back(3);
    heightVecVec.push_back(heightRowVec);

    heightRowVec.clear();
    heightRowVec.push_back(3); heightRowVec.push_back(7); heightRowVec.push_back(1);
    heightVecVec.push_back(heightRowVec);

    heightRowVec.clear();
    heightRowVec.push_back(4); heightRowVec.push_back(5); heightRowVec.push_back(2);
    heightVecVec.push_back(heightRowVec);

    ogl.setupHeightMesh(heightVecVec, 1.0f,
                        1.0f, 0.0f,
                        1.0f, 0.0f);

#if 1
    for (int rowIdx = 0; rowIdx < rowCnt; rowIdx++) {
      cout << endl << "Row " << rowIdx << ":\t";
      for (int colIdx = 0; colIdx < colCnt; colIdx++) {
        cout << res.at(static_cast<uint32_t>(rowIdx)).at(static_cast<uint32_t>(colIdx)) << "\t";
      }
    }
    cout << endl << endl;
#endif

    //ogl.doIndex();
    //ogl.loadIntoVBO();
    //ogl.enterLoop();
  }

  return 0;
}
