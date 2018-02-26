// insert license boilerplate
// provides a function that will connect to a postgres database and
// execute a SQL query

#include "query.h"
#include <pqxx/pqxx>

// not exception safe, catch exceptions in code that calls this function
pqxx::result query(const std::string & db, const std::string & sql) {
  pqxx::connection conn(db);
  pqxx::work transaction(conn);

  pqxx::result r = transaction.exec(sql);
  transaction.commit();
  return r;
}

