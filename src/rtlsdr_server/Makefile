

EXECUTABLE=rtlsdr_server


# Compilers
CXX=g++

# Compiler flags
CXXFLAGS=-I/usr/local/include/ -I../rtlsdrcpp/ -I../utilities/ -I../protobuffers/
LDLIBS=-L/usr/local/lib/ -lrtlsdr -lzmq -lfftw3 -pthread -lprotobuf

# Items
SRCS=../utilities/fft.cpp ../rtlsdrcpp/RTLSDRDevice.cpp main.cpp

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


