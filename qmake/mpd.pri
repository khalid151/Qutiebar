enablempd
{
    message(MPD enabled)
    LIBS += -lmpdclient
    DEFINES += ENABLE_MPD
    SOURCES += ../src/Utils/MpdConnection.cpp
    HEADERS += ../include/Utils/MpdConnection.h
}
