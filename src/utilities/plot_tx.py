
from matplotlib import pyplot
import numpy
import pandas

data = pandas.read_csv('tx.csv')

pyplot.plot(data.t, data.i)
pyplot.plot(data.t, data.q)
pyplot.show()
