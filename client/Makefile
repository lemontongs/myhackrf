

EXECUTABLE=test_client


# Compilers
CXX=g++ 
PROTOC = protoc

# Compiler flags
CXXFLAGS=-I. -I../protobuffers/
PROTOCFLAGS=--proto_path=../protobuffers/ --cpp_out=../protobuffers/
LDLIBS=-L/usr/local/lib/ -lprotobuf -lzmq -lpthread

# Items
SRCS=SDRReceiver.cpp main.cpp

OBJS=$(subst .cpp,.o,$(SRCS))
PROTOS=$(wildcard ../protobuffers/*.proto)
PBSRC=$(PROTOS:%.proto=%.pb.cc)
PBHDR=$(PROTOS:%.proto=%.pb.h)
PBOBJ=$(PROTOS:%.proto=%.pb.obj)

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


