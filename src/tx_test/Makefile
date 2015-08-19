

EXECUTABLE=txtest

CXXFLAGS=-I/usr/local/include/libhackrf/
LDLIBS=-lhackrf
#LDFLAGS=-L./
CXX=g++

SRCS=main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: %(EXECUTABLE)

%(EXECUTABLE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	$(RM) $(OBJS)
	$(RM) $(EXECUTABLE)
