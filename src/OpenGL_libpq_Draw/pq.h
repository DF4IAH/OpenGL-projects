#include <vector>
#include <string>
#include <libpq-fe.h>

using namespace std;

typedef vector< vector< string > > vvs_t;


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

  vvs_t execSync(const string params);


private:
  PGconn     *pqCon	= NULL;
  PGresult   *pqRes	= NULL;

  int         pqRowCnt  = 0U;
  int         pqColCnt  = 0;

};
