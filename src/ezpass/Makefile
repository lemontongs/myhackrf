

EXECUTABLE=ezpass_trigger

CXXFLAGS=-I/usr/local/include/libhackrf/ -I../hackrfcpp/
LDLIBS=-lhackrf
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
	$(RM) $(EXECUTABLE)
