#ifndef MISCFUNCTIONS_H
#define MISCFUNCTIONS_H

#include <QObject>
#include <QStringList>
#include <QColor>

namespace Utils
{
    QStringList arguments(QString); // Used to split text ignoring quotes. (parsing exec)
    QString section(const QString&, int, int); // Take a section of string, leaving rich-text.
    int rawStrLength(const QString&); // Length of string without tags.
    QColor getColor(const QString&); // In case rgba(int, int, int, float) is used. Returns Transparent color if nothing is valid.

    // Base class for Mpd\Mpris
    class MusicAdaptor
    {
        public:
            enum PlayerState{PAUSED, PLAYING, STOPPED, ERROR}; // ERROR for Mpd

            // Medial control
            virtual void playPause() = 0;
            virtual void play() = 0;
            virtual void pause() = 0;
            virtual void stop() = 0;
            virtual void next() = 0;
            virtual void previous() = 0;

            virtual PlayerState getState() = 0;
            virtual float getSongTimePercentage() = 0;
            virtual void setSongInfoFormat(const QString&) = 0;
            virtual QString getSongInfo() = 0;
    };

    // Base class for DisplayItem data "modules"
    class DataModel : public QObject
    {
        Q_OBJECT

        public:
            virtual bool hasState() { return false; }
            virtual int getState() { return 0; }
            virtual int getMax() { return 100; }
            virtual int getData() = 0;
            virtual QString getName() = 0;
            virtual QString getUnit() { return ""; }

        signals:
            void update();
    };
}

#endif // MISCFUNCTIONS_H
