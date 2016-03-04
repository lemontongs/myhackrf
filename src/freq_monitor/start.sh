#! /bin/bash

make

#
# Start the blink server
#
blink_server_pid=`ps -elf | grep blink_server.py | grep -v "grep" | awk '{print $4}'`
if [[ $blink_server_pid -eq "" ]]; then
    echo "Not running, starting..."
    ../blink_server/blink_server.py &
    blink_server_pid=$!
fi
echo "blink sever: $blink_server_pid"

#
# Start the hackrf server
#
# TODO: Add support for specifying an external server, python script sending
#       a test message expecting the INVALID response.
#
hackrf_server_pid=`ps -elf | grep hackrf_server | grep -v "grep" | awk '{print $4}'`
if [[ $hackrf_server_pid -eq "" ]]; then
    echo "Not running, starting..."
    ../hackrf_server/hackrf_server &
    hackrf_server_pid=$!
fi
echo "hackrf sever: $hackrf_server_pid"


./freq_monitor


