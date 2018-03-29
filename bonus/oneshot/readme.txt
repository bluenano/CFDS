Unix domain dgram-sockets IPC example.
C++ is server
PHP is client

first, run these commands, server daemon must be running before a client sends a message:

jw@jw-laptop ~/CS160Project/oneshot $ g++ -Wall -Wextra -std=c++11 -O2 -s server.cpp -o server.out
jw@jw-laptop ~/CS160Project/oneshot $ ./server.out 
can you process this for me? 0.mp4
exiting loop
jw@jw-laptop ~/CS160Project/oneshot $ 

then, after server is running, in a separate terminal:

jw@jw-laptop ~/CS160Project/oneshot $ php -f phpclient.php
received: its ready
jw@jw-laptop ~/CS160Project/oneshot $ 






