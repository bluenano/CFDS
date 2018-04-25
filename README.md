# Computer Face Detection System
Team 404

There is a file in our google drive folder that describes tasks for our project.
Feel free to add more tasks to this file.

Setting up the database:
login to psql
CREATE DATABASE cs160;
\c cs160
\i /path/to/src/CS160Project/sql/setup.sql

Building psycopg2:
install python3-dev, libpq-dev if not installed
try running pg_config --version
download src archive from initd.org and extract
inside src folder:
python3 setup.py build
sudo python3 setup.py install

Building libpqxx:
git clone https://github.com/jtv/libpqxx.git
inside src folder:
./configure --disable-documentation
make
sudo make install

Building C++ programs using libpqxx:
g++ -std=c++11 src.cpp -lpqxx -lpq

Note to Code Reviewers:

Our webpages are not fully routed yet.
We have not implemented the ability to play an uploaded video in
a webpage and the ability to delete an uploaded video.

Here is a description of some directories to make your review easier:  <br /> 
<br />
Front-End - html, css, js files <br /> 
api - php and configuration files for our backend infrastructure <br />
bin - script to setup our project in a directory recognized by apache <br />
sql - erd diagram and sql script to create our database tables <br />
src - test programs for database and video processing tasks <br />
vidproc - video processing code <br />
