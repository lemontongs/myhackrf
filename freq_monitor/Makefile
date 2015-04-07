

EXECUTABLE=freq_monitor

CXXFLAGS=-I/usr/local/include/libhackrf/ -I../hackrfcpp/
LDLIBS=-lzmq -lhackrf -lfftw3
#LDFLAGS=-L./
CXX=g++

SRCS=../hackrfcpp/HackRFDevice.cpp main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: %(EXECUTABLE)

%(EXECUTABLE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(EXECUTABLE)
