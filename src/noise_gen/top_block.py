#!/usr/bin/env python2
##################################################
# GNU Radio Python Flow Graph
# Title: Top Block
# Generated: Thu Nov 26 19:45:53 2015
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from PyQt4 import Qt
from gnuradio import analog
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from gnuradio.qtgui import Range, RangeWidget
from optparse import OptionParser
import osmosdr
import sys
import time


class top_block(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Top Block")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Top Block")
        try:
             self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
             pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "top_block")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())

        ##################################################
        # Variables
        ##################################################
        self.transition_width = transition_width = 50000
        self.samp_rate = samp_rate = 10e6
        self.low_cutoff = low_cutoff = 1
        self.high_cutoff = high_cutoff = 200000
        self.Fc = Fc = 915200000

        ##################################################
        # Blocks
        ##################################################
        self._transition_width_range = Range(0, 200000, 1, 50000, 200)
        self._transition_width_win = RangeWidget(self._transition_width_range, self.set_transition_width, "transition_width", "counter_slider", float)
        self.top_layout.addWidget(self._transition_width_win)
        self._low_cutoff_range = Range(0, 100000, 1, 1, 200)
        self._low_cutoff_win = RangeWidget(self._low_cutoff_range, self.set_low_cutoff, "low_cutoff", "counter_slider", float)
        self.top_layout.addWidget(self._low_cutoff_win)
        self._high_cutoff_range = Range(0, 500000, 1, 200000, 200)
        self._high_cutoff_win = RangeWidget(self._high_cutoff_range, self.set_high_cutoff, "high_cutoff", "counter_slider", float)
        self.top_layout.addWidget(self._high_cutoff_win)
        self._Fc_range = Range(0, 6e9, 1, 915200000, 200)
        self._Fc_win = RangeWidget(self._Fc_range, self.set_Fc, "Fc", "counter_slider", float)
        self.top_layout.addWidget(self._Fc_win)
        self.osmosdr_sink_0 = osmosdr.sink( args="numchan=" + str(1) + " " + "hackrf=1" )
        self.osmosdr_sink_0.set_sample_rate(samp_rate)
        self.osmosdr_sink_0.set_center_freq(Fc, 0)
        self.osmosdr_sink_0.set_freq_corr(0, 0)
        self.osmosdr_sink_0.set_gain(10, 0)
        self.osmosdr_sink_0.set_if_gain(20, 0)
        self.osmosdr_sink_0.set_bb_gain(0, 0)
        self.osmosdr_sink_0.set_antenna("0", 0)
        self.osmosdr_sink_0.set_bandwidth(0, 0)
          
        self.band_pass_filter_0 = filter.fir_filter_ccf(1, firdes.band_pass(
        	1, samp_rate, low_cutoff, high_cutoff, transition_width, firdes.WIN_HAMMING, 6.76))
        self.analog_noise_source_x_0 = analog.noise_source_c(analog.GR_GAUSSIAN, 1, 0)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_noise_source_x_0, 0), (self.band_pass_filter_0, 0))    
        self.connect((self.band_pass_filter_0, 0), (self.osmosdr_sink_0, 0))    

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "top_block")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_transition_width(self):
        return self.transition_width

    def set_transition_width(self, transition_width):
        self.transition_width = transition_width
        self.band_pass_filter_0.set_taps(firdes.band_pass(1, self.samp_rate, self.low_cutoff, self.high_cutoff, self.transition_width, firdes.WIN_HAMMING, 6.76))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.band_pass_filter_0.set_taps(firdes.band_pass(1, self.samp_rate, self.low_cutoff, self.high_cutoff, self.transition_width, firdes.WIN_HAMMING, 6.76))
        self.osmosdr_sink_0.set_sample_rate(self.samp_rate)

    def get_low_cutoff(self):
        return self.low_cutoff

    def set_low_cutoff(self, low_cutoff):
        self.low_cutoff = low_cutoff
        self.band_pass_filter_0.set_taps(firdes.band_pass(1, self.samp_rate, self.low_cutoff, self.high_cutoff, self.transition_width, firdes.WIN_HAMMING, 6.76))

    def get_high_cutoff(self):
        return self.high_cutoff

    def set_high_cutoff(self, high_cutoff):
        self.high_cutoff = high_cutoff
        self.band_pass_filter_0.set_taps(firdes.band_pass(1, self.samp_rate, self.low_cutoff, self.high_cutoff, self.transition_width, firdes.WIN_HAMMING, 6.76))

    def get_Fc(self):
        return self.Fc

    def set_Fc(self, Fc):
        self.Fc = Fc
        self.osmosdr_sink_0.set_center_freq(self.Fc, 0)


if __name__ == '__main__':
    parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
    (options, args) = parser.parse_args()
    from distutils.version import StrictVersion
    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        Qt.QApplication.setGraphicsSystem(gr.prefs().get_string('qtgui','style','raster'))
    qapp = Qt.QApplication(sys.argv)
    tb = top_block()
    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()
    tb = None  # to clean up Qt widgets
