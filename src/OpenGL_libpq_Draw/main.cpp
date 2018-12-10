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
  }

  return 0;
}
