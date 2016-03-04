

TX_EXECUTABLE=radar_tx
RX_EXECUTABLE=radar_rx

CXXFLAGS=-I/usr/local/include/libhackrf/ -I../hackrfcpp/
LDLIBS=-lhackrf
CXX=g++

TX_SRCS=../hackrfcpp/HackRFDevice.cpp main_tx.cpp
TX_OBJS=$(subst .cpp,.o,$(TX_SRCS))

RX_SRCS=../hackrfcpp/HackRFDevice.cpp main_rx.cpp
RX_OBJS=$(subst .cpp,.o,$(RX_SRCS))

all: %(TX_EXECUTABLE) %(RX_EXECUTABLE)

%(TX_EXECUTABLE): $(TX_OBJS)
	$(CXX) $(LDFLAGS) -o $(TX_EXECUTABLE) $(TX_OBJS) $(LDLIBS)

%(RX_EXECUTABLE): $(RX_OBJS)
	$(CXX) $(LDFLAGS) -o $(RX_EXECUTABLE) $(RX_OBJS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	$(RM) $(TX_EXECUTABLE)
	$(RM) $(RX_EXECUTABLE)

