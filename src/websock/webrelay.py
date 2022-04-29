#!/usr/bin/env python3

import asyncio
import websockets
import sys
sys.path.append('../../build/src/protobuffers/')
sys.path.append('../protobuffers/')
import myhackrf


radio = myhackrf.MyHackrf('localhost')
radio.send("set-rx-mode fft")
radio.send("set-fs 2000000")
radio.send("set-fc 101100000")
radio.send("set-rx-gain 40")


async def stream(websocket, uri):
    print('stream')

    while True:
        print('waiting for start')
        message = await websocket.recv()
        
        if message == "start":
            print('got start')

            # Receive Rx data from SDR
            try:
                while True:
                    # wait for a request for data
                    try:
                        print('waiting for request...')
                        message = await websocket.recv()
                        print('Got:', message)
                    except websockets.ConnectionClosedOK:
                        break
                    if message == "more":
                        # Get data
                        await websocket.send(radio.rx_sock.recv())
                    elif message == "stop":
                        break
            except KeyboardInterrupt:
                pass


# Workaround for ctrl-c signals....
def wakeup():
    loop.call_later(0.1, wakeup)


if __name__ == '__main__':
    loop = asyncio.get_event_loop()
    server = websockets.serve(stream, "0.0.0.0", 8765)
    loop.call_later(0.1, wakeup)
    
    try:
        loop.run_until_complete(server)
        print("run_forever")
        loop.run_forever()
    except KeyboardInterrupt:
        print("ctrl-c")
        loop.stop()
    finally:
        print("closing")
        loop.close()
