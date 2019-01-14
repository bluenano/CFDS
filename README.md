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
