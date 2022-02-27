import zmq
import packet_pb2
from matplotlib import pyplot

ctx = zmq.Context()
sock = ctx.socket(zmq.SUB)
sock.connect("tcp://localhost:5555")
sock.setsockopt(zmq.SUBSCRIBE, b"")

while True:
	msg = sock.recv()
	p = packet_pb2.Packet()
	p.ParseFromString(msg)

	print(p.num_bins)
	break

pyplot.plot(p.freq_bins_mhz, p.signal)
pyplot.show()

