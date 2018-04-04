#!/usr/bin/python3

import datetime
import psycopg2

conn = psycopg2.connect(host='localhost', dbname='cs160', user='postgres', password='asdf')
cur = conn.cursor()

#user_id = 666
width = 1920
height = 1080
frame_rate = 30
frame_count = 120

username = 'user'
password = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'
firstname = 'user'
lastname = 'user'
lastip = '127.0.0.1'
lastlogin = datetime.datetime.now()
sessionid = 'abcdabcdabcdabcdabcdabcdabcdabcd'

testUser = '''INSERT INTO userinfo (Username, Password, FirstName, LastName, LastIp, LastLogin, SessionID) VALUES (%s, %s, %s, %s, %s, %s, %s) RETURNING UserID;'''
testArg = username, password, firstname, lastname, lastip, lastlogin, sessionid
cur.execute(testUser, testArg)
user_id = str(cur.fetchone()[0])
print(user_id)

conn.commit()

sql = '''INSERT INTO Video (UserID, NumFrames, FramesPerSecond, Width, Height)
    VALUES (%s, %s, %s, %s, %s) RETURNING VideoID;'''
arg = user_id, frame_count, frame_rate, width, height
cur.execute(sql, arg)
video_id = str(cur.fetchone()[0])
print(video_id)

conn.commit()
cur.close()
conn.close()
