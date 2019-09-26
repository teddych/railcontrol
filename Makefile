
CC=gcc
CPP=g++
CCRASPI=aarch64-linux-gcc
CPPRASPI=aarch64-linux-g++

CPPFLAGS=-I. -g -O0 -Wall -Wextra -Werror -std=c++11
# -O2 does not work
CPPFLAGSAMALGAMATION=-I. -g -O0 -Wall -Wextra -Werror -std=c++11
CPPFLAGSRASPI=-I. -g -O0 -Wall -Wextra -Wno-cast-function-type -Werror -std=c++11 --sysroot=/home/teddy/buildroot-2018.11/output/host/aarch64-buildroot-linux-gnu/sysroot
LDFLAGS=-g -Wl,--export-dynamic
LIBS=-lpthread -ldl

OBJ= \
	DelayedCall.o \
	Logger/Logger.o \
	Logger/LoggerServer.o \
	config.o \
	datamodel/accessory.o \
	datamodel/feedback.o \
	datamodel/HardwareHandle.o \
	datamodel/layout_item.o \
	datamodel/LockableItem.o \
	datamodel/Loco.o \
	datamodel/object.o \
	datamodel/relation.o \
	datamodel/serializable.o \
	datamodel/signal.o \
	datamodel/street.o \
	datamodel/switch.o \
	datamodel/track.o \
	hardware/HardwareHandler.o \
	manager.o \
	network/Serial.o \
	network/TcpConnection.o \
	network/TcpServer.o \
	railcontrol.o \
	storage/StorageHandler.o \
	Timestamp.o \
	Utils/Utils.o \
	WebServer/HtmlFullResponse.o \
	WebServer/HtmlResponse.o \
	WebServer/HtmlResponseNotFound.o \
	WebServer/HtmlResponseNotImplemented.o \
	WebServer/HtmlTag.o \
	WebServer/HtmlTagAccessory.o \
	WebServer/HtmlTagButton.o \
	WebServer/HtmlTagButtonCancel.o \
	WebServer/HtmlTagButtonCommand.o \
	WebServer/HtmlTagButtonCommandToggle.o \
	WebServer/HtmlTagButtonOK.o \
	WebServer/HtmlTagButtonPopup.o \
	WebServer/HtmlTagFeedback.o \
	WebServer/HtmlTagInput.o \
	WebServer/HtmlTagInputInteger.o \
	WebServer/HtmlTagInputSlider.o \
	WebServer/HtmlTagInputSliderLocoSpeed.o \
	WebServer/HtmlTagSelect.o \
	WebServer/HtmlTagSignal.o \
	WebServer/HtmlTagStreet.o \
	WebServer/HtmlTagSwitch.o \
	WebServer/HtmlTagTrack.o \
	WebServer/Response.o \
	WebServer/WebClient.o \
	WebServer/WebServer.o

all: $(OBJ)
	make -C hardware
	make -C storage
	$(CPP) $(LDFLAGS) $(OBJ) -o railcontrol $(LIBS)
	rm Timestamp.h

amalgamation: Timestamp.h
	./amalgamation.bash
	$(CPP) $(CPPFLAGSAMALGAMATION) -DAMALGAMATION -c -o amalgamation.o amalgamation.cpp
	make -C storage amalgamation
	$(CPP) -g amalgamation.o storage/sqlite/sqlite3.o -o railcontrol $(LIBS)
	rm -f amalgamation.o
	rm -f amalgamation.cpp
	rm Timestamp.h

raspi: Timestamp.h
	./amalgamation.bash
	$(CPPRASPI) $(CPPFLAGSRASPI) -DAMALGAMATION -c -o amalgamation.o amalgamation.cpp
	make -C storage raspi
	$(CPPRASPI) -g amalgamation.o storage/sqlite/sqlite3.o -o railcontrol $(LIBS)
	rm -f amalgamation.o
	rm -f amalgamation.cpp
	rm Timestamp.h

sqlite-shell:
	make -C storage/sqlite

Timestamp.o: Timestamp.cpp Timestamp.h
	$(CPP) $(CPPFLAGS) -c -o $@ $<

%.o: %.cpp *.h console/*.h datamodel/*.h hardware/HardwareHandler.h Logger/*.h storage/StorageHandler.h text/*.h Utils/*.h WebServer/*.h
	$(CPP) $(CPPFLAGS) -c -o $@ $<

clean:
	make -C hardware clean
	make -C storage clean
	rm -f *.o console/*.o datamodel/*.o text/*.o Utils/*.o WebServer/*.o
	rm -f railcontrol

clean-sqlite-shell:
	make -C storage/sqlite clean

test:
	make -C test

Timestamp.h:
	echo "#pragma once" > Timestamp.h
	echo "#define __COMPILE_TIME__ `date +%s`" >> Timestamp.h
	echo "time_t GetCompileTime();" >> Timestamp.h
