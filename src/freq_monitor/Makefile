

EXECUTABLE=freq_monitor


# Compilers
CXX=g++

# Compiler flags
CXXFLAGS=-I../client/ -I../protobuffers/
LDLIBS=-L/usr/local/lib/ -lprotobuf -lzmq -pthread

# Items
SRCS=../client/SDRReceiver.cpp main.cpp

OBJS=$(subst .cpp,.o,$(SRCS))
PROTOLIB=../protobuffers/protobuffers.a

# Rules
all: %(EXECUTABLE)

%(EXECUTABLE): $(PROTOLIB) $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(PROTOLIB) $(LDLIBS)

$(PROTOLIB):
	$(MAKE) -C ../protobuffers

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	$(RM) $(PROTOLIB)
	$(RM) $(EXECUTABLE)


