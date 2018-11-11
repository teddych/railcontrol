
CC=g++

#CPPFLAGS=-g -O2 -Wall
#CPPFLAGS=-g -O0 -Wall -std=c++11
CPPFLAGS=-I. -g -O0 -Wall -std=c++11
#CPPFLAGS=-I. -g -O2 -Wall -std=c++11
LDFLAGS=-g -Wl,--export-dynamic
LIBS=-lpthread -ldl

OBJ= \
	config.o \
	console/console.o \
	datamodel/accessory.o \
	datamodel/block.o \
	datamodel/feedback.o \
	datamodel/layout_item.o \
	datamodel/loco.o \
	datamodel/object.o \
	datamodel/relation.o \
	datamodel/serializable.o \
	datamodel/street.o \
	datamodel/switch.o \
	DelayedCall.o \
	hardware/HardwareHandler.o \
	hardware/HardwareParams.o \
	manager.o \
	network/TcpConnection.o \
	network/TcpServer.o \
	railcontrol.o \
	storage/storage_handler.o \
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
	webserver/HtmlTagButtonOK.o \
	webserver/HtmlTagButtonPopup.o \
	webserver/HtmlTagInput.o \
	webserver/HtmlTagInputSlider.o \
	webserver/HtmlTagInputSliderLocoSpeed.o \
	webserver/HtmlTagSelect.o \
	webserver/webserver.o \
	webserver/webclient.o

all: $(OBJ)
	make -C hardware
	make -C storage
	$(CC) $(LDFLAGS) $(OBJ) -o railcontrol $(LIBS)

sqlite-shell:
	make -C storage/sqlite

%.o: %.cpp *.h datamodel/*.h console/*.h webserver/*.h storage/*.h hardware/*.h text/*.h
	$(CC) $(CPPFLAGS) -c -o $@ $<

clean:
	make -C hardware clean
	make -C storage clean
	rm -f *.o webserver/*.o datamodel/*.o console/*.o text/*.o
	rm -f railcontrol

clean-sqlite-shell:
	make -C storage/sqlite clean

test:
	make -C test
