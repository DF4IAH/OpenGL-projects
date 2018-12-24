#ifndef PQ_H_
#define PQ_H_

#include <vector>
#include <string>
#include <libpq-fe.h>

using namespace std;

typedef vector< vector< string > > vvs_t;
typedef vector< vector< uint64_t > > vvUI64_t;


class pq {
public:
  pq(void);

  ~pq();


  int getRowCnt(void);
  int getColCnt(void);


  bool connect(const string dbName);
  void disconnect(void);

  void clrRes(void);
  void clrCon(void);

  vvs_t execSyncVVS(const string params);
  vvUI64_t execSyncVVUI64(const string params);


private:
  PGconn     *pqCon	= NULL;
  PGresult   *pqRes	= NULL;

  int         pqRowCnt  = 0U;
  int         pqColCnt  = 0;

};

#endif
