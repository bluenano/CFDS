#include <iostream>
#include <pqxx/pqxx>

/*
    These are gathered now also; once at the start of the process.

    Again... not sure what do do with them...
*/
struct VideoMetadata
{
    int32_t nframes;
    int32_t fps;
    int32_t width;
    int32_t height;
};

struct alignas(4) PairInt16
{
    int16_t x16, y16;//2 bytes each

    int16_t x() const {return x16;}//dumb getters, was trying to fit it into dlib
    int16_t y() const {return y16;}
};


struct EulerAnglesF32
{
    float yaw, pitch, roll;//4 bytes each
};

/*
    One of these is filled in each frame.

    Some fields will be set to error conditions or
    unfilled/stale data if detection failed, see below.
*/
struct FrameResults//a POD struct
{
    PairInt16 marks68[68];  //if marks68[0].x==-1 then there was no detection,
                            //and nothing else is filled in for this entire object

    PairInt16 left_pupil;   //if .x==-1 then not found
    PairInt16 right_pupil;  //if .x==-1 then not found

    EulerAnglesF32 rotation;//if marks68 found, then these will be filled in
};

int main(int, char *argv[])
{
  pqxx::connection c("user=postgres host=localhost password=asdf dbname=cs160");

  //FrameResults* p_test = new FrameResults;
  FrameResults test;//= *p_test;

  int count = 1;
  for (int i=0;i<68;i++) {
    test.marks68[i].x = count;
    count++;
    test.marks68[i].y = count;
    count++;
  }

  test.left_pupil.x16 = 69;
  test.left_pupil.y16 = 70;
  test.right_pupil.x16 = 71;
  test.right_pupil.y16 = 72;

  test.rotation.yaw = 1.11;
  test.rotation.pitch = 2.22;
  test.rotation.roll = 3.33;


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
    txn.quote(test.rotation.roll) + ", " +
    txn.quote(test.rotation.pitch) + ", " +
    txn.quote(test.rotation.yaw) +
    ")"
  );

  pqxx::result r2 = txn.exec(
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
    txn.quote(test.left_pupil.x16) +
    ")"
  );

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
