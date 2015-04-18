

EXECUTABLE=test_client

CXXFLAGS=
LDLIBS=-lzmq -lpthread
LDFLAGS=
CXX=g++

SRCS=SDRReceiver.cpp main.cpp
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
