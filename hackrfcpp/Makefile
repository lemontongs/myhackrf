

EXECUTABLE=test_driver

CXXFLAGS=-I/usr/local/include/libhackrf/
LDLIBS=-lhackrf
#LDFLAGS=-L./
CXX=g++ 

SRCS=HackRFDevice.cpp test.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: %(EXECUTABLE)

%(EXECUTABLE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(EXECUTABLE)
