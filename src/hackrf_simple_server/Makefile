

EXECUTABLE=hackrf_simple_server


# Compilers
CXX=g++

# Compiler flags
CXXFLAGS=-I/usr/local/include/libhackrf/ -I../hackrfcpp/
LDLIBS=-L/usr/local/lib/ -lhackrf

# Items
SRCS=../hackrfcpp/HackRFDevice.cpp main.cpp Socket.cpp ServerSocket.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

# Rules
all: %(EXECUTABLE)

%(EXECUTABLE): $(PROTOLIB) $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(LDLIBS)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	$(RM) $(EXECUTABLE)


