#!/bin/bash

while :
do
    t="`TZ=":ROK" date +%Y_%m_%d_%H:%M:%S:%2N`"
    log_file="/home/ubuntu/storage/public/log/log_stream_server_${t}.txt"
    cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release >> $log_file
    cmake --build cmake-build-release --target stream_server -- -j 4 >> $log_file
    ./cmake-build-release/stream_server 7000 /home/ubuntu/storage >> ${log_file} 2>&1
    wait
done
