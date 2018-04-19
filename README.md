# Computer Face Detection System
Team 404

There is a file in our google drive folder that describes tasks for our project.
Feel free to add more tasks to this file.

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