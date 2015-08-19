
LIBRARY=protobuffers.a

# Compilers
CXX=g++
PROTOC = protoc

# Compiler flags
CXXFLAGS=-I.
PROTOCFLAGS=--proto_path=. --cpp_out=.

# Items
PROTOS=$(wildcard *.proto)
PBSRC=$(PROTOS:%.proto=%.pb.cc)
PBHDR=$(PROTOS:%.proto=%.pb.h)
PBOBJ=$(PROTOS:%.proto=%.pb.obj)

# Rules
all: %(LIBRARY)

%(LIBRARY): $(PBOBJ)
	ar -cvq $(LIBRARY) $(PBOBJ)

%.pb.obj: %.proto
	$(PROTOC) $(PROTOCFLAGS) $<
	$(CXX) $(CXXFLAGS) -c -o $*.pb.obj $*.pb.cc

clean:
	$(RM) $(PBOBJ)
	$(RM) $(PBSRC)
	$(RM) $(PBHDR)
	$(RM) $(LIBRARY)

