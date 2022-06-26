from matplotlib import pyplot
import myhackrf

dev = myhackrf.MyHackrf('localhost')
dev.send("set-fs 1024000")
dev.send("set-fc 462562500")
dev.send("set-fc 101100000")
dev.send("set-rx-gain 402")
dev.send("set-rx-mode fft")

waterfall = []
f = []
while len(waterfall) < 100:
    p = dev.recv()
    if p.header.type == myhackrf.packet_pb2.Packet_Header.FFT:
        f = p.fft_packet.freq_bins_hz
        waterfall.append(p.fft_packet.fft)
        print("GOT FFT " + str(len(waterfall)))

pyplot.figure()
pyplot.imshow(waterfall, extent=[min(f)/1e6, max(f)/1e6, 0, len(waterfall)], origin='lower', aspect='auto')
pyplot.colorbar()
pyplot.xlabel('Freq (MHz)')
pyplot.ylabel('Time')
pyplot.show()
