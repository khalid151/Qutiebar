enablealsa
{
    message(Alsa enabled)
    LIBS += -lasound
    DEFINES += ENABLE_ALSA
    SOURCES += ../src/Data/Volume.cpp
    HEADERS += ../include/Data/Volume.h
}
