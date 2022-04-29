import zmq
import packet_pb2

# Supported commands:
#    get-fc
#    get-fs
#    get-rx-gain
#    set-fc
#    set-fs
#    set-rx-gain
#    set-rx-mode iq|pdw|fft
#    quit

class MyHackrf:
    def __init__(self, host):
        ctx = zmq.Context()
        self.rx_sock = ctx.socket(zmq.SUB)
        self.rx_sock.connect("tcp://" + host + ":5555")
        self.rx_sock.setsockopt(zmq.SUBSCRIBE, b"")

        self.tx_sock = ctx.socket(zmq.REQ)
        self.tx_sock.connect("tcp://" + host + ":5556")

    def get_params(self):
        self.tx_sock.send_string("get-fc")
        print(self.tx_sock.recv())

    def send(self, cmd):
        self.tx_sock.send_string(cmd)
        rv = self.tx_sock.recv()
        print(cmd, ":", rv)
        return rv
        
    def recv(self):
        msg = self.rx_sock.recv()
        p = packet_pb2.Packet()
        p.ParseFromString(msg)
        return p
        

if __name__ == "__main__":
    from matplotlib import pyplot
    import numpy

    dev = MyHackrf('rpi4')
    dev.send("set-fs 2000000")
    dev.send("set-fc 915500000")
    dev.send("set-rx-gain 60")
    
    if 0:
        dev.send("set-rx-mode fft")
        
        waterfall = []
        f = []
        while len(waterfall) < 100:
            p = dev.recv()
            if p.header.type == packet_pb2.Packet_Header.FFT:
                f = p.fft_packet.freq_bins_hz
                waterfall.append(p.fft_packet.fft)
                print("GOT FFT " + str(len(waterfall)))
        
        pyplot.figure()
        pyplot.imshow(waterfall, extent=[min(f)/1e6, max(f)/1e6, 0, len(waterfall)], origin='lower', aspect='auto')
        pyplot.colorbar()
        pyplot.xlabel('Freq (MHz)')
        pyplot.ylabel('Time')
    
    if 1:
        dev.send("set-rx-mode pdw")
        
        while True:
            p = dev.recv()
            print(p.header.type)
            if p.header.type == packet_pb2.Packet_Header.PDW:
                print("GOT %d PDWs" % len(p.pdw_packet.pdws))
                for pdw in p.pdw_packet.pdws:
                    print("TOA: {0}  {1}  {2}  {3}  {4}  {5}".format(
                            ("%0.3f" % pdw.toa_s).rjust(10), 
                            ("%0.6f" % pdw.pw_s).rjust(10), 
                            ("%0.3f" % pdw.mean_amp_db).rjust(10), 
                            ("%0.3f" % pdw.peak_amp_db).rjust(10), 
                            ("%0.3f" % pdw.noise_amp_db).rjust(10), 
                            ("%0.3f" % pdw.freq_offset_hz).rjust(10))
                          )

        # pyplot.figure()
        
        # for pdw in p.pdw_packet.pdws:
        #     complex_sig = numpy.array(pdw.signal[0::2]) + 1j * numpy.array(pdw.signal[1::2])
        #     pyplot.plot(pdw.toa_s + numpy.array(range(len(complex_sig)))/8e6, 20.0*numpy.log10(numpy.abs(complex_sig)))
       
    if 0:
        dev.send("set-rx-mode iq")
        
        print("b4 recv")
        p = dev.recv()
        print("af recv")
        print(p.header.type)
        if p.header.type == packet_pb2.Packet_Header.IQ:
            print("GOT IQ", len(p.iq_packet.signal))
            complex_sig = numpy.array(p.iq_packet.signal[0::2]) + 1j * numpy.array(p.iq_packet.signal[1::2])
        
        pyplot.figure()
        pyplot.plot(numpy.array(range(len(complex_sig)))/8e6, 20.0*numpy.log10(numpy.abs(complex_sig)))
    
    
    pyplot.show()








