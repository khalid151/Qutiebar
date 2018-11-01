QT += core gui widgets
LIBS += -lxcb -lxcb-ewmh -lxcb-icccm
RESOURCES = ../res/resources.qrc

SOURCES += \
            ../src/Main.cpp \
            ../src/Utils/Builder.cpp \
            ../src/Utils/EventHandler.cpp \
            ../src/Utils/WidgetProperties.cpp \
            ../src/Utils/DBusHandler.cpp \
            ../src/Utils/X11EventHandler.cpp \
            ../src/Utils/UnixSignalHandler.cpp \
            ../src/Utils/TextScroller.cpp \
            ../src/Utils/Misc.cpp \
            ../src/Widgets/Panel.cpp \
            ../src/Widgets/Icon.cpp \
            ../src/Widgets/Text.cpp \
            ../src/Widgets/Progress.cpp \
            ../src/Modules/Desktops.cpp \
            ../src/Modules/AppIcon.cpp \
            ../src/Data/Battery.cpp \
            ../src/Data/Backlight.cpp \
            ../src/Modules/DisplayItem.cpp \
            ../src/Modules/Clock.cpp \
            ../src/Modules/MusicDisplay.cpp \
            ../src/Modules/Group.cpp
HEADERS += \
            ../include/Utils/Builder.h \
            ../include/Utils/EventHandler.h \
            ../include/Utils/WidgetProperties.h \
            ../include/Utils/DBusHandler.h \
            ../include/Utils/X11EventHandler.h \
            ../include/Utils/UnixSignalHandler.h \
            ../include/Utils/TextScroller.h \
            ../include/Utils/Misc.h \
            ../include/Widgets/Panel.h \
            ../include/Widgets/Icon.h \
            ../include/Widgets/Text.h \
            ../include/Widgets/Progress.h \
            ../include/Modules/Desktops.h \
            ../include/Modules/AppIcon.h \
            ../include/Data/Battery.h \
            ../include/Data/Backlight.h \
            ../include/Modules/DisplayItem.h \
            ../include/Modules/Clock.h \
            ../include/Modules/MusicDisplay.h \
            ../include/Modules/Group.h

INCLUDEPATH += ../include

TARGET = qutiebar

include(dbus.pri)
include(alsa.pri)
inlcude(mpd.pri)

CONFIG += x11 debug
