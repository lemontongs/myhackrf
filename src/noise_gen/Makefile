

EXECUTABLE=noise_gen


# Compilers
CXX=g++

# Compiler flags
CXXFLAGS=-I/usr/local/include/libhackrf/ -I/usr/local/include/gnuradio/filter/ -I../hackrfcpp/
LDLIBS=-L/usr/local/lib/ -lhackrf -lgnuradio-filter

# Items
SRCS=../hackrfcpp/HackRFDevice.cpp main.cpp

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


