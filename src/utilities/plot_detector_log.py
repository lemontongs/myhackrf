
from matplotlib import pyplot

import pandas

data = pandas.read_csv('detector.log')

pyplot.plot(data['index'], data['amp'])
pyplot.plot(data['index'], data['det_count'])
pyplot.plot(data['index'], data['noise_average'])
pyplot.plot(data['index'], data['state'])
pyplot.show()
