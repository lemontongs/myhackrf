#!/bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd $SCRIPT_DIR

protoc --proto_path=../protobuffers/ --js_out=binary:./js/ packet.proto

python2 -m SimpleHTTPServer &
HTTP_PID=$!

./webrelay.py

kill -SIGHUP $HTTP_PID

cd -
