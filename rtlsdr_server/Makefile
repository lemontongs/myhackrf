

EXECUTABLE=rtlsdr_server


# Compilers
CXX=g++
PROTOC = protoc

# Compiler flags
CXXFLAGS=-I/usr/local/include/ -I../rtlsdrcpp/ -I../utilities/ -I../protobuffers/
PROTOCFLAGS=--proto_path=../protobuffers/ --cpp_out=../protobuffers/
LDLIBS=-L/usr/local/lib/ -lrtlsdr -lzmq -lfftw3 -pthread -lprotobuf

# Items
SRCS=../utilities/fft.cpp ../rtlsdrcpp/RTLSDRDevice.cpp main.cpp

OBJS=$(subst .cpp,.o,$(SRCS))
PROTOS=$(wildcard ../protobuffers/*.proto)
PBSRC=$(PROTOS:%.proto=%.pb.cc)
PBHDR=$(PROTOS:%.proto=%.pb.h)
PBOBJ=$(PROTOS:%.proto=%.pb.obj)

# Rules
all: %(EXECUTABLE)

%(EXECUTABLE): $(PBOBJ) $(OBJS)
	$(CXX) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(PBOBJ) $(LDLIBS)

%.pb.obj: %.proto
	$(PROTOC) $(PROTOCFLAGS) $<
	$(CXX) $(CXXFLAGS) -c -o $*.pb.obj $*.pb.cc

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)
	$(RM) $(PBOBJ)
	$(RM) $(PBSRC)
	$(RM) $(PBHDR)
	$(RM) $(EXECUTABLE)

