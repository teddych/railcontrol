cmake_minimum_required(VERSION 3.1)

project(CMakeTest)

add_executable(railcontrol
Storage/sqlite/sqlite3.c
Manager.cpp
WebServer/WebClient.cpp
ArgumentHandler.cpp
Config.cpp
DataModel/AccessoryBase.cpp
DataModel/Accessory.cpp
DataModel/Cluster.cpp
DataModel/Feedback.cpp
DataModel/HardwareHandle.cpp
DataModel/LayoutItem.cpp
DataModel/LockableItem.cpp
DataModel/Loco.cpp
DataModel/LocoFunctions.cpp
DataModel/Object.cpp
DataModel/Relation.cpp
DataModel/Route.cpp
DataModel/Serializable.cpp
DataModel/Signal.cpp
DataModel/Switch.cpp
DataModel/TrackBase.cpp
DataModel/Track.cpp
Hardware/CcSchnitte.cpp
Hardware/CS2Tcp.cpp
Hardware/CS2Udp.cpp
Hardware/Ecos.cpp
Hardware/HardwareHandler.cpp
Hardware/Hsi88.cpp
Hardware/LocoCache.cpp
Hardware/M6051.cpp
Hardware/OpenDcc.cpp
Hardware/ProtocolMaerklinCAN.cpp
Hardware/RM485.cpp
Hardware/Virtual.cpp
Hardware/Z21.cpp
Hardware/ZLib.cpp
Languages.cpp
Logger/Logger.cpp
Logger/LoggerServer.cpp
Network/Serial.cpp
Network/TcpClient.cpp
Network/TcpConnection.cpp
Network/TcpServer.cpp
Network/UdpConnection.cpp
RailControl.cpp
Storage/Sqlite.cpp
Storage/StorageHandler.cpp
Timestamp.cpp
Utils/Utils.cpp
WebServer/HtmlFullResponse.cpp
WebServer/HtmlResponse.cpp
WebServer/HtmlResponseNotFound.cpp
WebServer/HtmlResponseNotImplemented.cpp
WebServer/HtmlTagAccessory.cpp
WebServer/HtmlTagButtonCancel.cpp
WebServer/HtmlTagButtonCommand.cpp
WebServer/HtmlTagButtonCommandPressRelease.cpp
WebServer/HtmlTagButtonCommandToggle.cpp
WebServer/HtmlTagButton.cpp
WebServer/HtmlTagButtonOK.cpp
WebServer/HtmlTagButtonPopup.cpp
WebServer/HtmlTag.cpp
WebServer/HtmlTagFeedback.cpp
WebServer/HtmlTagInput.cpp
WebServer/HtmlTagInputInteger.cpp
WebServer/HtmlTagInputSlider.cpp
WebServer/HtmlTagInputSliderLocoSpeed.cpp
WebServer/HtmlTagRoute.cpp
WebServer/HtmlTagSelect.cpp
WebServer/HtmlTagSignal.cpp
WebServer/HtmlTagSwitch.cpp
WebServer/HtmlTagTrackBase.cpp
WebServer/Response.cpp
WebServer/WebClientCluster.cpp
WebServer/WebClientSignal.cpp
WebServer/WebClientTrackBase.cpp
WebServer/WebClientTrack.cpp
WebServer/WebServer.cpp
Hardware/zlib/adler32.c
Hardware/zlib/compress.c
Hardware/zlib/crc32.c
Hardware/zlib/deflate.c
Hardware/zlib/gzclose.c
Hardware/zlib/gzlib.c
Hardware/zlib/gzread.c
Hardware/zlib/gzwrite.c
Hardware/zlib/infback.c
Hardware/zlib/inffast.c
Hardware/zlib/inflate.c
Hardware/zlib/inftrees.c
Hardware/zlib/trees.c
Hardware/zlib/uncompr.c
Hardware/zlib/zutil.c
)

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-I. -g -O2 -Wall -Wextra -pedantic -Werror")
set(CMAKE_C_FLAGS "-g -O2 -DSQLITE_ENABLE_FTS4 -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_RTREE -DHAVE_USLEEP -Wno-implicit-function-declaration")
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(railcontrol PRIVATE Threads::Threads dl)
