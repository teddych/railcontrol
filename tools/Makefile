
#CPPFLAGS=-g -O2 -Wall
#CPPFLAGS=-g -O0 -Wall -std=c++11
CXXFLAGS=-I. -I.. -g -O0 -Wall -std=c++11
LDFLAGS=-g
LIBS=-lpthread -ldl

TOOLS= \
	Cc-Schnitte-Sniffer

OBJ= \
	Cc-Schnitte-Sniffer.o \
	../Network/Serial.o \
	../Network/TcpConnection.o \
	../Network/TcpServer.o \
	../Logger/Logger.o \
	../Logger/LoggerServer.o \
	../Languages.o \
	../Utils/Utils.o

all: $(TOOLS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Cc-Schnitte-Sniffer: $(OBJ)
	$(CXX) $(LDFLAGS) -o Cc-Schnitte-Sniffer Cc-Schnitte-Sniffer.o ../Logger/Logger.o ../Logger/LoggerServer.o ../Network/Serial.o ../Network/TcpServer.o ../Network/TcpConnection.o ../Languages.o ../Utils/Utils.o $(LIBS)

clean:
	rm -f $(TESTS) *.o

