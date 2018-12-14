#include <iostream>
#include <string>

#include "ogl.h"
#include "pq.h"

using namespace std;


int main(int argc, char* argv[])
{
  ogl ogl;
  pq  db;

  if (db.connect("gis")) {
    cout << "main: DB gis ok." << endl;
    vvs_t res = db.execSync("SELECT *  FROM TOPO  WHERE id = 1  AND  row between 5000 and 5000  LIMIT 100;");
    int rowCnt = db.getRowCnt();
    int colCnt = db.getColCnt();

    cout << "main: Got " << rowCnt << " rows with " << colCnt << " columns." << endl;

#if 1
    for (int rowIdx = 0; rowIdx < rowCnt; rowIdx++) {
      cout << endl << "Row " << rowIdx << ":\t";
      for (int colIdx = 0; colIdx < colCnt; colIdx++) {
        cout << res.at(rowIdx).at(colIdx) << "\t";
      }
    }
    cout << endl << endl;
#endif
  }

  return 0;
}
