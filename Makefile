# should be either OSC_HOST_BIG_ENDIAN or OSC_HOST_LITTLE_ENDIAN
# Apple Mac OS X: OSC_HOST_BIG_ENDIAN
# Win32: OSC_HOST_LITTLE_ENDIAN
# i386 GNU/Linux: OSC_HOST_LITTLE_ENDIAN

TARGET = main
CPPSOURCES = $(shell ls *.cpp)
PPOBJECTS = $(CPPSOURCES:.cpp=.o)
OSCSOURCES = ./oscpack/osc/OscTypes.cpp ./oscpack/osc/OscOutboundPacketStream.cpp ./oscpack/osc/OscReceivedElements.cpp ./oscpack/osc/OscPrintReceivedElements.cpp ./oscpack/ip/posix/NetworkingUtils.cpp ./oscpack/ip/posix/UdpSocket.cpp
OSCOBJECTS = $(OSCSOURCES:.cpp=.o)
TUIOSOURCES = ./TUIO/TuioClient.cpp ./TUIO/TuioServer.cpp ./TUIO/TuioTime.cpp
TUIOOBJECTS = $(TUIOSOURCES:.cpp=.o)
INCLUDES = -Ioscpack
CFLAGS = -m32 -DLINUX -DOSC_HOST_LITTLE_ENDIAN -DNDEBUG -DPMALSA $(INCLUDES)
CXXFLAGS = $(CFLAGS)

all: $(TARGET)

install: $(TARGET)
	cp libsmt-linux/libSMT.so /usr/lib/
	cp main /usr/bin/statum-tuio

$(TARGET): $(CPPOBJECTS) $(OSCOBJECTS) $(TUIOOBJECTS) main.cpp
	g++ $(CXXFLAGS) -Llibsmt-linux -lSMT -lpthread $(OSCOBJECTS) $(TUIOOBJECTS) -o $@ main.cpp

clean:
	rm -f $(TARGET) $(CPPOBJECTS) $(OSCOBJECTS) $(TUIOOBJECTS)
