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
    vvs_t res = db.execSync("SELECT *  FROM geonamesorg  WHERE name = 'Ladenburg';");
    int rowCnt = db.getRowCnt();
    int colCnt = db.getColCnt();

    for (int rowIdx = 0; rowIdx < rowCnt; rowIdx++) {
      cout << endl << "Row" << rowIdx << "\t";
      for (int colIdx = 0; colIdx < colCnt; colIdx++) {
        cout << res.at(rowIdx).at(colIdx) << "\t";
      }
    }
    cout << endl << endl;
  }

  return 0;
}
