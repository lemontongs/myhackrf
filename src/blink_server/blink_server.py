#! /usr/bin/env python

import os
import sys
import signal
import zmq

def signal_handler(signal, frame):
    print('Blink server exiting')
    os.system("blink1-tool -q -m 0 --off")
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

port = "5558"
if len(sys.argv) > 1:
    port =  sys.argv[1]
    int(port)

context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.bind("tcp://*:%s" % port)
socket.setsockopt(zmq.SUBSCRIBE, "blink")

while True:
    try:
        msg = socket.recv()
    except KeyboardInterrupt:
        signal_handler(0,0)
    
    # remove the topic
    args = msg.split()
    topic = args.pop(0)
    
    for arg in args:
        if arg == "on":
            os.system("blink1-tool -q -m 0 --hsb=130,200,80")
        if arg == "off":
            os.system("blink1-tool -q -m 0 --off")
            

