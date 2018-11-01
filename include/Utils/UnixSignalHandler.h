#ifndef UNIXSIGNALHANDLER_H
#define UNIXSIGNALHANDLER_H

#include <QSocketNotifier>

namespace Utils
{
    class UnixSignalHandler : public QObject
    {
        Q_OBJECT

        public:
            UnixSignalHandler(QObject* = nullptr);

            static void sigintHandler(int);
            static void sigtermHandler(int);

        private:
            static int sigintFd[2], sigtermFd[2];
            QSocketNotifier *snInt, *snTerm;

        public slots:
            void handleSigint();
            void handleSigterm();

        signals:
            void stop();
    };
}

#endif // UNIXSIGNALHANDLER_H
