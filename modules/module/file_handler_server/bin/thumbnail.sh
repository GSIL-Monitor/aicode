#!/bin/bash

if [ $# -ne 2 ]; then
    echo "usage:   $0 inputvideo resolution"
    echo "example: $0 video.mp4 1280x720"
    exit -1
fi

filepath=$1
resolution=$2
video=${1}.mp4

if [ -s "$video" ]; then
    date +%x-%H:%M:%S >>thumb.log
    ./ffmpeg -i $video -y -f mjpeg -vframes 1 -s $resolution ${filepath}_${resolution}.jpg 1>>thumb.log 2>&1
    echo "" >>thumb.log
else
    echo "input video is empty"
    exit -1
fi
