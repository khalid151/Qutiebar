#include "Utils/Mpris.h"

namespace Utils
{
    bool Mpris::hasInstance = false;

    Mpris::Mpris(const QString &service, QObject *parent)
        : QDBusAbstractInterface(service, objectPath(), interface(), QDBusConnection::sessionBus(), parent)
    {
        // PropertiesChanged is emitted by music player when metadata is changed.
        // Connecting it to emitSongChanged(), that will emit another signal.
        // Because PropertiesChanged is on another interface. Also,
        // QDBusConnection doesn't (apparently) have the "new" signals synatx.
        QDBusConnection::sessionBus().connect(service, objectPath(), properties(), "PropertiesChanged", this, SLOT(emitSongChanged()));

        // Updating maximum song length on songChanged()
        songLength = metadata().value("mpris:length").toFloat();
        connect(this, &Mpris::songChanged, this, [this](){ songLength = metadata().value("mpris:length").toFloat(); });

        hasInstance = true;
    }

    // Media controls
    void
    Mpris::playPause()
    {
        call("PlayPause");
    }

    void
    Mpris::play()
    {
        call("Play");
    }

    void
    Mpris::pause()
    {
        call("Pause");
    }

    void
    Mpris::stop()
    {
        call("Stop");
    }

    void
    Mpris::next()
    {
        call("Next");
    }

    void
    Mpris::previous()
    {
        call("Previous");
    }

    MusicAdaptor::PlayerState
    Mpris::getState()
    {
        if(playbackStatus() == "Playing")
            return MusicAdaptor::PlayerState::PLAYING;
        else if(playbackStatus() == "Paused")
            return MusicAdaptor::PlayerState::PAUSED;
        else
            return MusicAdaptor::PlayerState::STOPPED;
    }

    float
    Mpris::getSongTimePercentage()
    {
        return position()/songLength;
    }

    void
    Mpris::setSongInfoFormat(const QString &format)
    {
        this->format = format;
    }

    QString
    Mpris::getSongInfo()
    {
        auto title = metadata().value("xesam:title").toString();
        auto artist = metadata().value("xesam:artist").toStringList()[0];
        auto album = metadata().value("xesam:album").toString();
        auto str = QString(format);
        str.replace("<artist>", artist);
        str.replace("<title>", title);
        str.replace("<album>", album);
        return str;
    }

    // Private
    void
    Mpris::emitSongChanged()
    {
        emit songChanged();
    }
}
