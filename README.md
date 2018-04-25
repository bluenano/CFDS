# Computer Face Detection System
Team 404

Setting up the database: <br />
login to psql <br />
CREATE DATABASE cs160; <br />
\c cs160 <br />
\i /path/to/src/CS160Project/sql/setup.sql . <br />
<br />
Building psycopg2: <br />
install python3-dev, libpq-dev if not installed <br />
try running pg_config --version <br />
download src archive from initd.org and extract <br />
inside src folder: <br />
python3 setup.py build <br />
sudo python3 setup.py install <br />
<br />
Building libpqxx: <br />
git clone https://github.com/jtv/libpqxx.git <br />
inside src folder: <br />
./configure --disable-documentation <br />
make <br />
sudo make install <br />

Building C++ programs using libpqxx: <br />
g++ -std=c++11 src.cpp -lpqxx -lpq <br />
<br />
Note to Code Reviewers: <br />
<br />
Our webpages are not fully routed yet.<br />
We have not implemented the ability to play an uploaded video in <br />
a webpage and the ability to delete an uploaded video. <br />
<br />
Here is a description of some directories to make your review easier:  <br /> 
<br />
Front-End - html, css, js files <br /> 
api - php and configuration files for our backend infrastructure <br />
bin - script to setup our project in a directory recognized by apache <br />
sql - erd diagram and sql script to create our database tables <br />
src - test programs for database and video processing tasks <br />
vidproc - video processing code <br />
