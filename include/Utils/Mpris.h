#ifndef MPRIS_H
#define MPRIS_H

#include <QDBusAbstractInterface>
#include <QDBusConnection>
#include <QString>
#include <QObject>
#include <QVariant>

#include "Utils/Misc.h" // For base class

namespace Utils
{
    class Mpris : public QDBusAbstractInterface, public MusicAdaptor
    {
        Q_OBJECT
        Q_PROPERTY(QVariantMap Metadata READ metadata)
        Q_PROPERTY(qlonglong Position READ position)
        Q_PROPERTY(QString PlaybackStatus READ playbackStatus)

        public:
            Mpris(const QString &service, QObject* = nullptr);

            static bool hasInstance; // No need to create another instance to add more modules

            void playPause();
            void play();
            void pause();
            void stop();
            void next();
            void previous();

            MusicAdaptor::PlayerState getState();
            float getSongTimePercentage();
            void setSongInfoFormat(const QString&);
            QString getSongInfo();

        private:
            float songLength; // to hold maximum length of song
            QString format; // song format, will be substitued with info on getSongInfo()

            static inline const char *properties()
            { return "org.freedesktop.DBus.Properties"; }
            static inline const char *interface()
            { return "org.mpris.MediaPlayer2.Player"; }
            static inline const char *objectPath()
            { return "/org/mpris/MediaPlayer2"; }

            inline QVariantMap metadata() const {
                return qvariant_cast< QVariantMap >(property("Metadata"));
            }
            inline qlonglong position() const {
                return property("Position").toLongLong();
            }
            inline QString playbackStatus() const {
                return property("PlaybackStatus").toString();
            }

        private slots:
            void emitSongChanged(); // Signal emitted by music player

        signals:
            void songChanged();
    };
}

#endif // MPRIS_H
