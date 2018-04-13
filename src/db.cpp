#include <iostream>
#include <pqxx/pqxx>

//typedef dlib::point lmCoord;//landmark coordinate pair

//typedef lmCoord image_coords_xy;

struct alignas(4) PairInt16
{
    int16_t x16, y16;//2 bytes each

    int16_t x() const {return x16;}
    int16_t y() const {return y16;}
};

//typedef dlib::point lmCoord;//landmark coordinate pair

//typedef xy_coords_int16 lmCoord;

struct EulerAnglesF32
{
    float yaw, pitch, roll;//4 bytes each
};

//note error conditions
struct FrameResults//a POD struct
{
    PairInt16 marks68[68];//if marks68[0].x==-1 then was an error, known where found (no pupils either)

    PairInt16 left_pupil;//if .x==-1 not found
    PairInt16 right_pupil;//if .x==-1 not found
    //70 * 4
    EulerAnglesF32 rot;//+ 4*3 //if marks found, always found
};

int main(int, char *argv[])
{
  pqxx::connection c("user=postgres host=localhost password=asdf dbname=cs160");

  FrameResults* p_test = new FrameResults;
  FrameResults test = *p_test;

  /*int count = 1;
  for (int i=0;i<68;i++) {
    test.marks68[i].x = count;
    count++;
    test.marks68[i].y = count;
    count++;
  }*/

  test.left_pupil.x16 = 69;
  test.left_pupil.y16 = 70;
  test.right_pupil.x16 = 71;
  test.right_pupil.y16 = 72;

  test.rot.yaw = 1.11;
  test.rot.pitch = 2.22;
  test.rot.roll = 3.33;


  pqxx::work txn(c);

  pqxx::result r1 = txn.exec(
    "INSERT INTO frame("
    "videoid, "
    "ftpupilrightx, "
    "ftpupilrighty, "
    "ftpupilleftx, "
    "ftpupillefty, "
    "roll, "
    "pitch, "
    "yaw)"
    "VALUES (" +
    txn.quote(4) + ", " +
    txn.quote(test.right_pupil.x16) + ", " +
    txn.quote(test.right_pupil.y16) + ", " +
    txn.quote(test.left_pupil.x16) + ", " +
    txn.quote(test.right_pupil.x16) + ", " +
    txn.quote(test.rot.roll) + ", " +
    txn.quote(test.rot.pitch) + ", " +
    txn.quote(test.rot.yaw) +
    ")"
  );

  /*pqxx::result r2 = txn.exec(
    "INSERT INTO frame("
    "videoid, "
    "ftpupilrightx, "
    "ftpupilrighty, "
    "ftpupilleftx, "
    "ftpupillefty, "
    "roll, "
    "pitch, "
    "yaw)"
    "VALUES (" +
    txn.quote(4) + ", " +
    txn.quote(test.right_pupil.x16) + ", " +
    txn.quote(test.right_pupil.y16) + ", " +
    txn.quote(test.left_pupil.x16) + ", " +
    txn.quote(test.right_pupil.x16) + ", " +
    txn.quote(test.rot.roll) + ", " +
    txn.quote(test.rot.pitch) + ", " +
    txn.quote(test.rot.yaw) +
    ")"
  );*/

  /*pqxx::result r = txn.exec(
    "SELECT width "
    "FROM video "
    "WHERE videoid = 3" + txn.quote(argv[1]));

  int width = r[0][0].as<int>();
  std::cout << "Width = " << width << std::endl;*/

  /*txn.exec(
    "UPDATE EMPLOYEE "
    "SET salary = salary + 1 "
    "WHERE id = " + txn.quote(employee_id));*/

  txn.commit();
}
