#!/usr/bin/python3

import os
import sys
import subprocess

VIDEO_DIR = '/home/user/github/'
VIDEO_ID = 'vid.mp4'
FRAMES_DIR = '/home/user/github/vid_frames/'

cmd = ['/usr/bin/ffprobe', '-count_frames', '-select_streams', 'v:0', '-show_entries',
    'stream=height,width,avg_frame_rate,nb_read_frames', '-of', 'csv=p=0:nk=1', VIDEO_DIR + VIDEO_ID]

width, height, frame_rate, frame_count = subprocess.check_output(cmd, universal_newlines=True).split(',')

print(width)
print(height)
print(frame_rate)
print(frame_count)

try:
    os.makedirs(FRAMES_DIR, 0o777)
except OSError:
        pass

cmd = ['/usr/bin/ffmpeg', '-i', VIDEO_DIR + VIDEO_ID, FRAMES_DIR + '/%d.png']

subprocess.check_call(cmd)

cmd = ['/usr/bin/ffmpeg', '-r', frame_rate, '-i', FRAMES_DIR + '/%d.png',
   '-c:v', 'libvpx-vp9', '-b:v', '1M', VIDEO_DIR + VIDEO_ID + '.webm']

subprocess.check_call(cmd)