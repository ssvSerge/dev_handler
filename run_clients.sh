#!/bin/bash

CLI_CNT=10;

for i in $(seq 1 $CLI_CNT); do

    ids=$(tr -cd '[:alnum:]' < /dev/urandom | head -c6)
    cnt=$(tr -cd '[:digit:]' < /dev/urandom | head -c3)
    per=$(tr -cd '[:digit:]' < /dev/urandom | head -c3)
    
    ./udp_client --port 1234 --name $ids --cnt $cnt --period $per &
    
    pids[${i}]=$!
    
done


for pid in ${pids[*]}; do
    wait $pid
done


