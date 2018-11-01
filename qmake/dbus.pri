enabledbus
{
    message(DBus enabled)
    QT += dbus
    DEFINES += ENABLE_DBUS
    SOURCES += ../src/Utils/Mpris.cpp
    HEADERS += ../include/Utils/Mpris.h
}
