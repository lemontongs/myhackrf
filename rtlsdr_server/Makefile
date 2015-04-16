

EXECUTABLE=rtlsdr_server

CXXFLAGS=-I/usr/local/include/ -I../rtlsdrcpp/
LDLIBS=-lrtlsdr -lzmq -lpthread
#LDFLAGS=-L./
CXX=g++

SRCS=../rtlsdrcpp/RTLSDRDevice.cpp main.cpp
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
