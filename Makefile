
CFLAGSSQLITE=-g -O2 -DSQLITE_ENABLE_FTS4 -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP
CXXFLAGS=-I. -g -O2 -Wall -Wextra -pedantic -Werror -std=c++11
CXXFLAGSAMALGAMATION=-I. -g -O2 -Wall -Wextra -Werror -std=c++11
LDFLAGS=-g -Wl,--export-dynamic
LIBS=-lpthread -ldl
LIBSAMALGAMATION=-lpthread -ldl

TMPDIR=RailControl
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
	Hardware/HardwareHandler.o \
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
	Storage/StorageHandler.o \
	Storage/Sqlite.o \
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

CXXUNSORTED= $(subst .o,.cpp,$(OBJ))
CXXSORTED= $(shell ls -S $(CXXUNSORTED))
OBJSORTED= Timestamp.o Storage/sqlite/sqlite3.o $(subst .cpp,.o,$(CXXSORTED))

all: $(OBJSORTED)
	+make -C Hardware
	rm Timestamp.cpp
	$(CXX) $(LDFLAGS) $(OBJSORTED) -o railcontrol $(LIBS)
	rm Timestamp.o

dist: all
	strip railcontrol
	strip Hardware/*.so
	tar cvJf railcontrol.tar.xz Hardware/*.so railcontrol railcontrol.conf.dist html/*
	mkdir $(TMPDIR)
	mkdir $(TMPDIR)/Hardware
	cp -r \
		html \
		railcontrol.conf.dist \
		railcontrol \
		$(TMPDIR)
	cp -r Hardware/*.so $(TMPDIR)/Hardware
	tar cvJf railcontrol.`date +"%Y%m%d"`.tar.xz $(TMPDIR)/* $(TMPDIR)/html/*
	rm -r $(TMPDIR)

dist-cygwin: amalgamation
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
	make -C Hardware amalgamation
	$(CXX) -g amalgamation.o Storage/sqlite/sqlite3.o Hardware/zlib/*.o -o railcontrol $(LIBSAMALGAMATION)
	rm -f amalgamation.o
	rm -f amalgamation.cpp
	rm Timestamp.cpp

sqlite-shell:
	make -C Storage/sqlite

Timestamp.o: Timestamp.cpp Timestamp.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

Storage/sqlite/sqlite3.o: Storage/sqlite/sqlite3.c Storage/sqlite/sqlite3.h
	$(CC) $(CFLAGSSQLITE) -c -o $@ $<

%.o: %.cpp *.h DataModel/*.h Hardware/HardwareHandler.h Logger/*.h Network/*.h Storage/StorageHandler.h Utils/*.h WebServer/*.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	make -C Hardware clean
	rm -f *.o DataModel/*.o Hardware/*.o Hardware/zlib/*.o Logger/*.o Network/*.o Storage/*.o Utils/*.o WebServer/*.o
	rm -f railcontrol

clean-sqlite-shell:
	make -C Storage/sqlite clean

test:
	make -C test

tools:
	make -C tools

Timestamp.cpp:
	echo "#include <ctime>" > Timestamp.cpp
	echo "#include \"Timestamp.h\"" >> Timestamp.cpp
	echo "time_t GetCompileTime() { return `date +%s`; }" >> Timestamp.cpp

