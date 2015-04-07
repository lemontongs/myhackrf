#! /bin/bash


blink_server_pid=`ps -elf | grep blink_server.py | grep -v "grep" | awk '{print $4}'`

if [[ $blink_server_pid -ne "" ]]; then
    # Kill the process
    if [[ $1 == "-f" ]]; then
        kill -9 $blink_server_pid
    else
        kill -2 $blink_server_pid
    fi
fi

hackrf_server_pid=`ps -elf | grep hackrf_server | grep -v "grep" | awk '{print $4}'`

if [[ $hackrf_server_pid -ne "" ]]; then
    # Kill the process
    if [[ $1 == "-f" ]]; then
        kill -9 $hackrf_server_pid
    else
        kill -2 $hackrf_server_pid
    fi
fi

