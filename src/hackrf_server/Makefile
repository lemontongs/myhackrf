

EXECUTABLE=hackrf_server


# Compilers
CXX=g++

# Compiler flags
CXXFLAGS=-I/usr/local/include/libhackrf/ -I../hackrfcpp/ -I../utilities/ -I../protobuffers/
LDLIBS=-L/usr/local/lib/ -lhackrf -lzmq -lfftw3 -lprotobuf

# Items
SRCS=../utilities/fft.cpp ../hackrfcpp/HackRFDevice.cpp main.cpp

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


