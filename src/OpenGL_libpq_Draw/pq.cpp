#include <iostream>
#include <string>
#include <libpq-fe.h>

#include "pq.h"

using namespace std;


pq::pq(void)
{
  cout << "PQ CON" << endl;
}

pq::~pq()
{
  cout << "PQ DESC" << endl;
  PQclear(res);
  PQfinish(conn);
}

bool pq::connect(const string dbName)
{
  string dbCon("dbname = ");
  dbCon.append(dbName);

  conn = PQconnectdb(dbCon.c_str());
  if (PQstatus(conn) == CONNECTION_OK) {
    /* Set always-secure search path, so malicious users can't take control. */
    res = PQexec(conn, "SELECT pg_catalog.set_config('search_path', '', false)");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
      cerr << "SET failed: " << PQerrorMessage(conn) << endl;
    }
    PQclear(res);
    return true;

  } else {
    cerr << "PQ: can not establish connection to the database" << dbName << endl;
    return false;
  }
}
