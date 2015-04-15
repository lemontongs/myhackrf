

EXECUTABLE=test_driver

CXXFLAGS=-I/usr/local/include/
LDLIBS=-lrtlsdr -lpthread
#LDFLAGS=-L./
CXX=g++ 

SRCS=RTLSDRDevice.cpp test_rtl.cpp
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
