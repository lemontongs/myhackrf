
all:
	$(MAKE) -C src/client
	$(MAKE) -C src/freq_monitor
	$(MAKE) -C src/hackrfcpp
	$(MAKE) -C src/hackrf_server
	$(MAKE) -C src/rtlsdrcpp
	$(MAKE) -C src/rtlsdr_server
	$(MAKE) -C src/tx_test
	qmake src/GUI/GUI.pro -o src/GUI/Makefile
	$(MAKE) -C src/GUI

clean:
	$(MAKE) -C src/client clean
	$(MAKE) -C src/freq_monitor clean
	$(MAKE) -C src/hackrfcpp clean
	$(MAKE) -C src/hackrf_server clean
	$(MAKE) -C src/rtlsdrcpp clean
	$(MAKE) -C src/rtlsdr_server clean
	$(MAKE) -C src/tx_test clean
	$(MAKE) -C src/GUI distclean

	
