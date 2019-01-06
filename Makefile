
CC=gcc
CPP=g++
CCRASPI=aarch64-linux-gcc
CPPRASPI=aarch64-linux-g++

CPPFLAGS=-I. -g -O0 -Wall -std=c++11
CPPFLAGSRASPI=-I. -g -O0 -Wall -std=c++11 --sysroot=/home/teddy/buildroot-2018.11/output/host/aarch64-buildroot-linux-uclibc/sysroot
LDFLAGS=-g -Wl,--export-dynamic
LIBS=-lpthread -ldl

OBJ= \
	config.o \
	console/ConsoleClient.o \
	console/ConsoleServer.o \
	datamodel/accessory.o \
	datamodel/feedback.o \
	datamodel/layout_item.o \
	datamodel/loco.o \
	datamodel/object.o \
	datamodel/relation.o \
	datamodel/serializable.o \
	datamodel/street.o \
	datamodel/switch.o \
	datamodel/track.o \
	DelayedCall.o \
	hardware/HardwareHandler.o \
	Logger/Logger.o \
	Logger/LoggerServer.o \
	manager.o \
	network/TcpConnection.o \
	network/TcpServer.o \
	railcontrol.o \
	storage/StorageHandler.o \
	text/converters.o \
	util.o \
	webserver/Response.o \
	webserver/HtmlFullResponse.o \
	webserver/HtmlResponse.o \
	webserver/HtmlResponseNotFound.o \
	webserver/HtmlResponseNotImplemented.o \
	webserver/HtmlTag.o \
	webserver/HtmlTagAccessory.o \
	webserver/HtmlTagButton.o \
	webserver/HtmlTagButtonCancel.o \
	webserver/HtmlTagButtonCommand.o \
	webserver/HtmlTagButtonCommandToggle.o \
	webserver/HtmlTagButtonOK.o \
	webserver/HtmlTagButtonPopup.o \
	webserver/HtmlTagInput.o \
	webserver/HtmlTagInputSlider.o \
	webserver/HtmlTagInputSliderLocoSpeed.o \
	webserver/HtmlTagSelect.o \
	webserver/HtmlTagSwitch.o \
	webserver/HtmlTagTrack.o \
	webserver/webserver.o \
	webserver/webclient.o

all: $(OBJ)
	make -C hardware
	make -C storage
	$(CPP) $(LDFLAGS) $(OBJ) -o railcontrol $(LIBS)

amalgamation:
	./amalgamation.bash
	$(CPP) $(CPPFLAGS) -DAMALGAMATION -c -o amalgamation.o amalgamation.cpp
	make -C storage amalgamation
	$(CPP) -g amalgamation.o storage/sqlite/sqlite3.o -o railcontrol $(LIBS)
	rm -f amalgamation.o
	rm -f amalgamation.cpp

raspi:
	./amalgamation.bash
	$(CPPRASPI) $(CPPFLAGSRASPI) -DAMALGAMATION -c -o amalgamation.o amalgamation.cpp
	make -C storage raspi
	$(CPPRASPI) -g amalgamation.o storage/sqlite/sqlite3.o -o railcontrol $(LIBS)
	rm -f amalgamation.o
	rm -f amalgamation.cpp

sqlite-shell:
	make -C storage/sqlite

%.o: %.cpp *.h console/*.h datamodel/*.h hardware/*.h Logger/*.h storage/*.h text/*.h webserver/*.h
	$(CPP) $(CPPFLAGS) -c -o $@ $<

clean:
	make -C hardware clean
	make -C storage clean
	rm -f *.o webserver/*.o datamodel/*.o console/*.o text/*.o
	rm -f railcontrol

clean-sqlite-shell:
	make -C storage/sqlite clean

test:
	make -C test
