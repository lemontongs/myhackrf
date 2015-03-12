

EXECUTABLE=client

CXXFLAGS=
LDLIBS=-lzmq
LDFLAGS=
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

distclean: clean
	$(RM) $(EXECUTABLE)
