from matplotlib import pyplot
import numpy
import myhackrf

dev = myhackrf.MyHackrf('localhost')
dev.send("set-fs 1024000")
#dev.send("set-fc 462562500")
dev.send("set-fc 101100000")
dev.send("set-rx-gain 297")
dev.send("set-rx-mode iq")


p = dev.recv()
if p.header.type == myhackrf.packet_pb2.Packet_Header.IQ:
    print("GOT IQ", len(p.iq_packet.signal))
    complex_sig = numpy.array(p.iq_packet.signal[0::2]) + 1j * numpy.array(p.iq_packet.signal[1::2])

pyplot.figure()
pyplot.plot(complex_sig.real[:100000], '.-')
pyplot.plot(complex_sig.imag[:100000], '.-')
#pyplot.plot(numpy.array(range(len(complex_sig)))/1024000, 20.0*numpy.log10(numpy.abs(complex_sig)))

pyplot.show()
