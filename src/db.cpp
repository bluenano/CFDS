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
    test.marks68[i].x16 = count;
    count++;
    test.marks68[i].y16 = count;
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

  int video_id = 1;
  int framenumber = 5;

  pqxx::result r1 = txn.exec(
    "INSERT INTO frame("
    "videoid, "
    "framenumber, "
    "ftpupilrightx, "
    "ftpupilrighty, "
    "ftpupilleftx, "
    "ftpupillefty, "
    "roll, "
    "pitch, "
    "yaw)"
    "VALUES (" +
    txn.quote(video_id) + ", " +
    txn.quote(framenumber) + ", " +
    txn.quote(test.right_pupil.x16) + ", " +
    txn.quote(test.right_pupil.y16) + ", " +
    txn.quote(test.left_pupil.x16) + ", " +
    txn.quote(test.left_pupil.y16) + ", " +
    txn.quote(test.rotation.roll) + ", " +
    txn.quote(test.rotation.pitch) + ", " +
    txn.quote(test.rotation.yaw) +
    ") RETURNING frameid"
  );

  int frameid = r1[0][0].as<int>();

  for (int i=0;i<68;i++) {
    pqxx::result r2 = txn.exec(
      "INSERT INTO openfacedata("
      "pointnumber, "
      "x, "
      "y, "
      "frameid)"
      "VALUES (" +
      txn.quote(i + 1) + ", " +
      txn.quote(test.marks68[i].x16) + ", " +
      txn.quote(test.marks68[i].y16) + ", " +
      txn.quote(frameid) +
      ")"
    );
    
  }

  txn.commit();
}
