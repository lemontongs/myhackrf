

EXECUTABLE=test_driver


# Compilers
CXX=g++

# Compiler flags
CXXFLAGS=-I/usr/local/include/libhackrf/ -I.
LDLIBS=-lhackrf

# Items
SRCS=HackRFDevice.cpp test_hackrf.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

# Rules
all: %(EXECUTABLE)

%(EXECUTABLE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	$(RM) $(EXECUTABLE)


