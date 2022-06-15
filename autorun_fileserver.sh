while :
do
    t="`TZ=":ROK" date +%Y_%m_%d_%H:%M:%S:%2N`"
    log_file="/home/ubuntu/storage/public/log/log_file_server_${t}.txt"
    python3 ~/Documents/GitHub/asio-server/file_server.py 8000 >> ${log_file} 2>&1
    wait
done