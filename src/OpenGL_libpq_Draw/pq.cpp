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
  disconnect();
}


int pq::getRowCnt(void)
{
  return pqRowCnt;
}

int pq::getColCnt(void)
{
  return pqColCnt;
}


bool pq::connect(const string dbName)
{
  string dbCon;
  dbCon.append("host=localhost port=5432 ");
  dbCon.append("dbname=");
  dbCon.append(dbName);
  dbCon.append(" user=wwwrun password=wwwrun");

  pqCon = PQconnectdb(dbCon.c_str());
  if (PQstatus(pqCon) == CONNECTION_OK) {
#if 0
    /* Set always-secure search path, so malicious users can't take control. */
    pqRes = PQexec(pqCon, "SELECT pg_catalog.set_config('search_path', '', false)");
    if (PQresultStatus(pqRes) != PGRES_TUPLES_OK) {
      cerr << "SET failed: " << PQerrorMessage(pqCon) << endl;
    }
    PQclear(pqRes);
#endif
    return true;

  } else {
    cerr << "PQ: can not establish connection to the database '" << dbName << "'" << endl;
    return false;
  }
}

void pq::disconnect(void)
{
  clrRes();
  clrCon();
}


void pq::clrRes(void)
{
  if (pqRes) {
    PQclear(pqRes);
    pqRes = 0;

    pqRowCnt = 0;
    pqColCnt = 0;
  }
}

void pq::clrCon(void)
{
  if (pqCon) {
    PQfinish(pqCon);
    pqCon = 0;
  }
}


vvs_t pq::execSync(const string params)
{
  vvs_t vvs;

  switch (PQstatus(pqCon))
  {
  case CONNECTION_OK:
    break;

  default:
    cerr << "pq execSync() connection error: PQstatus " << PQstatus(pqCon) << endl;
    return vvs;
  }

  clrRes();
  if (!PQsendQuery(pqCon, params.c_str())) {
    cerr << "pq execSync() query error: " << PQerrorMessage(pqCon) << endl;
    return vvs;
  }
  PQsetSingleRowMode(pqCon);

  while ((pqRes = PQgetResult(pqCon))) {
    ExecStatusType est = PQresultStatus(pqRes);

    switch (est)
    {
    case PGRES_TUPLES_OK:
      break;

    case PGRES_COPY_OUT:
    case PGRES_SINGLE_TUPLE:
      {
        //int rowCnt = PQntuples(pqRes);
        pqColCnt = PQnfields(pqRes);
        //cout << "execSync(): pulling row " << pqRowCnt << " with " << pqColCnt << " columns." << endl;

        vector<string> vs;

        for (int colIdx = 0; colIdx < pqColCnt; colIdx++) {
          vs.push_back(string(PQgetvalue(pqRes, 0, colIdx)));
        }

        vvs.push_back(vs);
        pqRowCnt++;
      }
      break;

    default:
      cout << "execSync(): " << PQresStatus(est) << endl;
    }
  }
  return vvs;
}
