#ifndef MPDCONNECTION_H
#define MPDCONNECTION_H

#include <mpd/client.h>
#include <QString>

#include "Utils/Misc.h"

namespace Utils
{
    class MpdConnection : public MusicAdaptor
    {
        public:
            MpdConnection(const QString&, int, const QString&, int);
            ~MpdConnection();

            static bool hasInstance; // No need to create another instance to add more modules
            bool couldConnect();

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
            bool isConnected = false;
            int port, timeout;
            QString host, password, format = "";
            mpd_connection *conn{};
            mpd_status *status{};
            mpd_song *song{};
            void mpdConnect(const QString&, int, const QString&, int);
            bool checkMpdConnection();
    };
}

#endif // MPDCONNECTION_H
