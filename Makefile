
CFLAGSSQLITE=-g -O2 -DSQLITE_ENABLE_FTS4 -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP
CFLAGSZLIB=-g -O2 -Wno-implicit-function-declaration
CXXFLAGS=-I. -g -O2 -Wall -Wextra -pedantic -Werror -std=c++11
CXXFLAGSAMALGAMATION=-I. -g -O2 -Wall -Wextra -Werror -std=c++11
LDFLAGS=-g
LIBS=-lpthread -ldl
LIBSAMALGAMATION=-lpthread -ldl

TMPDIR=/tmp/RailControl
TMPDIRCYGWIN=/RailControl

OBJ= \
	ArgumentHandler.o \
	Config.o \
	DataModel/Accessory.o \
	DataModel/AccessoryBase.o \
	DataModel/Cluster.o \
	DataModel/Feedback.o \
	DataModel/HardwareHandle.o \
	DataModel/LayoutItem.o \
	DataModel/LockableItem.o \
	DataModel/Loco.o \
	DataModel/LocoFunctions.o \
	DataModel/Object.o \
	DataModel/Relation.o \
	DataModel/Route.o \
	DataModel/Serializable.o \
	DataModel/Signal.o \
	DataModel/Switch.o \
	DataModel/Track.o \
	DataModel/TrackBase.o \
	Hardware/CS2Tcp.o \
	Hardware/CS2Udp.o \
	Hardware/CcSchnitte.o \
	Hardware/Ecos.o \
	Hardware/HardwareHandler.o \
	Hardware/Hsi88.o \
	Hardware/LocoCache.o \
	Hardware/M6051.o \
	Hardware/OpenDcc.o \
	Hardware/ProtocolMaerklinCAN.o \
	Hardware/RM485.o \
	Hardware/Virtual.o \
	Hardware/Z21.o \
	Languages.o \
	Logger/Logger.o \
	Logger/LoggerServer.o \
	Manager.o \
	Network/Serial.o \
	Network/TcpClient.o \
	Network/TcpConnection.o \
	Network/TcpServer.o \
	Network/UdpConnection.o \
	RailControl.o \
	Storage/Sqlite.o \
	Storage/StorageHandler.o \
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
	WebServer/HtmlTagButtonCommandPressRelease.o \
	WebServer/HtmlTagButtonCommandToggle.o \
	WebServer/HtmlTagButtonOK.o \
	WebServer/HtmlTagButtonPopup.o \
	WebServer/HtmlTagFeedback.o \
	WebServer/HtmlTagInput.o \
	WebServer/HtmlTagInputInteger.o \
	WebServer/HtmlTagInputSlider.o \
	WebServer/HtmlTagInputSliderLocoSpeed.o \
	WebServer/HtmlTagRoute.o \
	WebServer/HtmlTagSelect.o \
	WebServer/HtmlTagSignal.o \
	WebServer/HtmlTagSwitch.o \
	WebServer/HtmlTagTrackBase.o \
	WebServer/Response.o \
	WebServer/WebClient.o \
	WebServer/WebClientCluster.o \
	WebServer/WebClientSignal.o \
	WebServer/WebClientTrack.o \
	WebServer/WebClientTrackBase.o \
	WebServer/WebServer.o

OBJZLIB=Hardware/zlib/adler32.o \
	Hardware/zlib/compress.o \
	Hardware/zlib/crc32.o \
	Hardware/zlib/deflate.o \
	Hardware/zlib/gzclose.o \
	Hardware/zlib/gzlib.o \
	Hardware/zlib/gzread.o \
	Hardware/zlib/gzwrite.o \
	Hardware/zlib/infback.o \
	Hardware/zlib/inffast.o \
	Hardware/zlib/inflate.o \
	Hardware/zlib/inftrees.o \
	Hardware/zlib/trees.o \
	Hardware/zlib/uncompr.o \
	Hardware/zlib/zutil.o \
	Hardware/ZLib.o


CXXUNSORTED= $(subst .o,.cpp,$(OBJ))
CXXSORTED= $(shell ls -S $(CXXUNSORTED))
OBJSORTED= Timestamp.o Storage/sqlite/sqlite3.o $(subst .cpp,.o,$(CXXSORTED)) $(OBJZLIB)

all: $(OBJSORTED)
	rm Timestamp.cpp
	$(CXX) $(LDFLAGS) $(OBJSORTED) -o railcontrol $(LIBS)
	rm Timestamp.o

dist: all
	strip railcontrol
	mkdir $(TMPDIR)
	cp -r \
		html \
		railcontrol.conf.dist \
		railcontrol \
		$(TMPDIR)
	( cd $(TMPDIR)/.. && tar cvJf railcontrol.`date +"%Y%m%d"`.tar.xz RailControl/* RailControl/html/* )
	rm -r $(TMPDIR)

dist-cygwin: all
	strip railcontrol.exe
	mkdir $(TMPDIRCYGWIN)
	cp -r \
		/cygdrive/c/Windows/SYSTEM32/ntdll.dll \
		/cygdrive/c/Windows/system32/KERNELBASE.dll \
		/cygdrive/c/Windows/system32/kernel32.dll \
		/usr/bin/cyggcc_s-seh-1.dll \
		/usr/bin/cygstdc++-6.dll \
		/usr/bin/cygwin1.dll \
		html \
		railcontrol.conf.dist \
		railcontrol.exe \
		$(TMPDIRCYGWIN)
	zip -9 railcontrol.windows.`date +"%Y%m%d"`.zip $(TMPDIRCYGWIN)/* $(TMPDIRCYGWIN)/html/*
	rm -r $(TMPDIRCYGWIN)

amalgamation: Timestamp.cpp
	./amalgamation.bash
	$(CXX) $(CXXFLAGSAMALGAMATION) -DAMALGAMATION -c -o amalgamation.o amalgamation.cpp
	$(CXX) -g amalgamation.o Storage/sqlite/sqlite3.o Hardware/zlib/*.o -o railcontrol $(LIBSAMALGAMATION)
	rm -f amalgamation.o
	rm -f amalgamation.cpp
	rm Timestamp.cpp

sqlite-shell:
	make -C Storage/sqlite

Timestamp.o: Timestamp.cpp Timestamp.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Hardware/zlib/%.o: Hardware/zlib/%.c Hardware/zlib/*.h
	$(CC) $(CFLAGSZLIB) -c -o $@ $<

Storage/sqlite/sqlite3.o: Storage/sqlite/sqlite3.c Storage/sqlite/sqlite3.h
	$(CC) $(CFLAGSSQLITE) -c -o $@ $<

%.o: %.cpp *.h DataModel/*.h Hardware/*.h Logger/*.h Network/*.h Storage/*.h Utils/*.h WebServer/*.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f *.o DataModel/*.o Hardware/*.o Hardware/zlib/*.o Logger/*.o Network/*.o Storage/*.o Storage/sqlite/*.o Utils/*.o WebServer/*.o
	rm -f railcontrol

clean-sqlite-shell:
	make -C Storage/sqlite clean

test:
	make -C test

tools:
	make -C tools

.PHONY: Timestamp.cpp
Timestamp.cpp:
	echo "#include <ctime>" > Timestamp.cpp
	echo "#include \"Timestamp.h\"" >> Timestamp.cpp
	echo "time_t GetCompileTime() { return `date +%s`; }" >> Timestamp.cpp

