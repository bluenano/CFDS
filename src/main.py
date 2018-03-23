#!/usr/bin/python3

import os
import psycopg2
import subprocess
import sys

UPLOAD_DIR = '/home/user/github/'
UPLOAD_FILENAME = 'vid.mp4'
FRAMES_DIR = '/home/user/github/vid_frames/'
OUTPUT_DIR = '/home/user/github/output/'

cmd = ['/usr/bin/ffprobe', '-v', 'quiet', '-count_frames', '-select_streams', 'v:0', '-show_entries',
    'stream=height,width,avg_frame_rate,nb_read_frames', '-of', 'csv=p=0:nk=1', VIDEO_DIR + VIDEO_ID]

width, height, frame_rate, frame_count = subprocess.check_output(cmd, universal_newlines=True).split(',')

conn = psycopg2.connect(host='localhost', dbname='DB', user='USER', password='PASS')
cur = conn.cursor()
sql = '''INSERT INTO video (frame_count, frame_rate, width, height)
    VALUES (%s, %s, %s, %s) RETURNING video_id;'''
arg = frame_count, frame_rate, width, height
cur.execute(sql, arg)
video_id = str(cur.fetchone()[0])
conn.commit()
cur.close()
conn.close()

os.rename(UPLOAD_FILENAME, video_id)

#try:
#    os.makedirs(FRAMES_DIR, 0o777)
#except OSError:
#        pass

#try:
#    os.makedirs(UPLOAD_DIR, 0o777)
#except OSError:
#        pass

cmd = ['/usr/bin/ffmpeg', '-v', 'quiet', '-i', UPLOAD_DIR + video_id, FRAMES_DIR + video_id + '.%d.png']

subprocess.check_call(cmd)

cmd = ['/usr/bin/ffmpeg', '-v', 'quiet', '-r', frame_rate, '-i', FRAMES_DIR + video_id + '.%d.png',
   '-c:v', 'libvpx-vp9', '-b:v', '1M', OUTPUT_DIR + video_id + '.webm']

subprocess.check_call(cmd)