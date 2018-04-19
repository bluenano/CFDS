#!/usr/bin/python3

import datetime
import psycopg2

conn = psycopg2.connect(host='localhost', dbname='cs160', user='postgres', password='asdf')
cur = conn.cursor()

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

testUser = '''INSERT INTO userinfo (username, password, firstname, lastname, lastip, lastlogin, sessionid) VALUES (%s, %s, %s, %s, %s, %s, %s) RETURNING userid;'''
testArg = username, password, firstname, lastname, lastip, lastlogin, sessionid
cur.execute(testUser, testArg)
user_id = str(cur.fetchone()[0])
print("USER ID: " + user_id)

conn.commit()

sql = '''INSERT INTO video (userid, numframes, framespersecond, width, height)
    VALUES (%s, %s, %s, %s, %s) RETURNING videoid;'''
arg = user_id, frame_count, frame_rate, width, height
cur.execute(sql, arg)
video_id = str(cur.fetchone()[0])
print("VIDEO ID: " + video_id)

conn.commit()
cur.close()
conn.close()
