#include <iostream>
#include <pqxx/pqxx>

int main(int, char *argv[])
{
  pqxx::connection c("user=postgres host=localhost password=asdf dbname=cs160");
  pqxx::work txn(c);

  pqxx::result r = txn.exec(
    "SELECT width "
    "FROM video "
    "WHERE videoid = 3" /*+ txn.quote(argv[1])*/);

  int width = r[0][0].as<int>();
  std::cout << "Width = " << width << std::endl;

  /*txn.exec(
    "UPDATE EMPLOYEE "
    "SET salary = salary + 1 "
    "WHERE id = " + txn.quote(employee_id));*/

  txn.commit();
}
