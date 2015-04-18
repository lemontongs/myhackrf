

EXECUTABLE=freq_monitor

CXXFLAGS=-I../client/
LDLIBS=-lzmq -lfftw3 -lpthread
#LDFLAGS=-L./
CXX=g++

SRCS=../client/SDRReceiver.cpp main.cpp
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
