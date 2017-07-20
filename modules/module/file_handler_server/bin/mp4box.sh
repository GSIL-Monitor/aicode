#!/bin/bash

if [ $# -ne 3 ] && [ $# -ne 4 ]; then
    echo "usage:   $0 inputvideo [inputaudio] fps outputvideo"
    echo "example: $0 video.h264 audio.aac 15 out.mp4"
    echo "         $0 video.h264 15 out.mp4"
    exit -1
fi

if [ $# -eq 3 ]; then
    video=$1
    framerate=$2
    output=$3

    if [ -s "$video" ]; then
        ./MP4Box -add $video -fps $framerate -new $output
    else
        echo "input video is empty"
        exit -1
    fi
else
    video=$1
    audio=$2
    framerate=$3
    output=$4

    param=""

    if [ ! -s "$video" ] && [ ! -s "$audio" ]; then
        echo "input video and audio are both empty"
        exit -1
    fi

    if [ -s "$video" ]; then
        param+="-add $video "
    fi

    if [ -s "$audio" ]; then
        param+="-add $audio "
    fi

    param+="-fps $framerate -new $output"

    ./MP4Box $param
fi
