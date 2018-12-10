#include <string>
#include <libpq-fe.h>

using namespace std;


class pq {
public:
  pq(void);

  ~pq();

  bool connect(const string dbName);


private:
  PGconn     *conn	= NULL;
  PGresult   *res	= NULL;

};

