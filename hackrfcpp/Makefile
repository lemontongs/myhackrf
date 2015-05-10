

EXECUTABLE=test_driver


# Compilers
CXX=g++ 
PROTOC = protoc

# Compiler flags
CXXFLAGS=-I/usr/local/include/libhackrf/ -I.
LDLIBS=-lhackrf

# Items
SRCS=HackRFDevice.cpp test_hackrf.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
PROTOS=$(wildcard protobuffers/*.proto)
PBSRC=$(PROTOS:%.proto=%.pb.cc)
PBHDR=$(PROTOS:%.proto=%.pb.h)
PBOBJ=$(PROTOS:%.proto=%.pb.obj)

all: %(EXECUTABLE)

%(EXECUTABLE): $(OBJS) $(PBOBJ)
	$(CXX) -o $(EXECUTABLE) $(OBJS) $(PBOBJ) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

%.pb.obj: %.proto
	$(PROTOC) --cpp_out=. $<
	${CXX} ${CXXFLAGS} -c -o $*.pb.obj $*.pb.cc

clean:
	$(RM) $(OBJS)
	$(RM) $(PBOBJ)
	$(RM) $(PBSRC)
	$(RM) $(PBHDR)
	$(RM) $(EXECUTABLE)

