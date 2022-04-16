
from matplotlib import pyplot
import numpy

f = open('rx.bin', 'rb')
data = f.read()
f.close()

iq = numpy.array([int(d) for d in data]).astype(numpy.int8)
sig = iq[0::2] + 1j*iq[1::2]

pyplot.plot(20.0*numpy.log10(numpy.abs(sig)))
pyplot.show()
