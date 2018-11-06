#include <cstdio>
#include <QTimer>

#include "Utils/MpdConnection.h"

namespace Utils
{
    bool MpdConnection::hasInstance = false;

    MpdConnection::MpdConnection(const QString &host, int port, const QString &pass, int timeout)
    {
        mpdConnect(host, port, pass, timeout);
        hasInstance = true;
    }

    MpdConnection::~MpdConnection()
    {
        mpd_connection_free(conn);
    }

    bool
    MpdConnection::couldConnect()
    {
        return isConnected;
    }

    // Media controls
    void
    MpdConnection::playPause()
    {
        mpd_run_toggle_pause(conn);
    }

    void
    MpdConnection::play()
    {
        mpd_run_play(conn);
    }

    void
    MpdConnection::pause()
    {
        mpd_run_pause(conn, true);
    }

    void
    MpdConnection::stop()
    {
        mpd_run_stop(conn);
    }

    void
    MpdConnection::next()
    {
        mpd_run_next(conn);
    }

    void
    MpdConnection::previous()
    {
        mpd_run_previous(conn);
    }

    MusicAdaptor::PlayerState
    MpdConnection::getState()
    {
        if(!checkMpdConnection())
            return MusicAdaptor::PlayerState::ERROR;

        mpd_status *status = nullptr;
        mpd_command_list_begin(conn, true);
        mpd_send_status(conn);
        mpd_command_list_end(conn);

        if((status = mpd_recv_status(conn)) == nullptr)
        {
            mpd_response_finish(conn);
            return MusicAdaptor::PlayerState::ERROR;
        }

        switch(mpd_status_get_state(status))
        {
            case MPD_STATE_PLAY:
                mpd_response_finish(conn);
                mpd_status_free(status);
                return MusicAdaptor::PlayerState::PLAYING;
                break;
            case MPD_STATE_PAUSE:
                mpd_response_finish(conn);
                mpd_status_free(status);
                return MusicAdaptor::PlayerState::PAUSED;
                break;
            case MPD_STATE_STOP:
                mpd_response_finish(conn);
                mpd_status_free(status);
                return MusicAdaptor::PlayerState::STOPPED;
                break;
            default:
                mpd_response_finish(conn);
                mpd_status_free(status);
                return MusicAdaptor::PlayerState::ERROR;
                break;
        }
    }

    float
    MpdConnection::getSongTimePercentage()
    {
        if(!checkMpdConnection())
            return 0.0f;

        mpd_status *status = nullptr;
        mpd_command_list_begin(conn, true);
        mpd_send_status(conn);
        mpd_command_list_end(conn);

        if((status = mpd_recv_status(conn)) == nullptr)
        {
            mpd_response_finish(conn);
            return 0.0f;
        }

        float pos = static_cast<float>(mpd_status_get_elapsed_time(status))/mpd_status_get_total_time(status);
        mpd_response_finish(conn);
        mpd_status_free(status);
        return pos;
    }

    void
    MpdConnection::setSongInfoFormat(const QString &f)
    {
        format = f;
    }

    QString
    MpdConnection::getSongInfo()
    {
        if(!checkMpdConnection())
            return "";

        mpd_song *song = nullptr;
        mpd_command_list_begin(conn, true);
        mpd_send_current_song(conn);
        mpd_command_list_end(conn);

        if((song = mpd_recv_song(conn)) == nullptr)
        {
            mpd_response_finish(conn);
            return "";
        }

        const char *artist, *title, *album;
        artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
        title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
        album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
        mpd_response_finish(conn);
        auto str = QString(format);
        str.replace("<artist>", QString::fromUtf8(artist));
        str.replace("<title>", QString::fromUtf8(title));
        str.replace("<album>", QString::fromUtf8(album));
        mpd_song_free(song);
        return str;
    }

    // Private
    void
    MpdConnection::mpdConnect(const QString &host, int port, const QString &password, int timeout)
    {
        conn = mpd_connection_new(host.toLatin1().data(), port, timeout);
        if(!password.isEmpty())
            mpd_run_password(conn, password.toLatin1().data());

        if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
        {
            fprintf(stderr, "Could not connect to mpd.\n");
            return;
        }
        this->host = host;
        this->port = port;
        this->password = password;
        this->timeout = timeout;
        MpdConnection::isConnected = true;
    }

    bool
    MpdConnection::checkMpdConnection()
    {
        if(mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
        {
            if(isConnected)
            {
                isConnected = false;
                mpd_connection_free(conn);
                mpdConnect(host, port, password, timeout);
            }
             return false;
        }
        return true;
    }
}
