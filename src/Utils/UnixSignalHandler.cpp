#include <cstdio>
#include <csignal>
#include <sys/socket.h>
#include <unistd.h>

#include "Utils/UnixSignalHandler.h"

namespace Utils
{
    int UnixSignalHandler::sigintFd[2];
    int UnixSignalHandler::sigtermFd[2];

    static int setup_unix_signal_handlers()
    {
        struct sigaction sigint, sigterm;
        sigint.sa_handler = Utils::UnixSignalHandler::sigintHandler;
        sigterm.sa_handler = Utils::UnixSignalHandler::sigtermHandler;
        sigemptyset(&sigint.sa_mask);
        sigemptyset(&sigterm.sa_mask);
        sigint.sa_flags = 0;
        sigterm.sa_flags = 0;
        sigint.sa_flags |= SA_RESTART;
        sigterm.sa_flags |= SA_RESTART;

        if(sigaction(SIGINT, &sigint, nullptr))
            return 1;

        if(sigaction(SIGTERM, &sigterm, nullptr))
            return 2;

        return 0;
    }

    UnixSignalHandler::UnixSignalHandler(QObject *parent) : QObject(parent)
    {
        if(socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd))
            fprintf(stderr, "Couldn't create SIGINT socketpair.\n");
        if(socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
            fprintf(stderr, "Couldn't create SIGTERM socketpair.\n");
        snInt = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
        snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
        connect(snInt, &QSocketNotifier::activated, this, &UnixSignalHandler::handleSigint);
        connect(snTerm, &QSocketNotifier::activated, this, &UnixSignalHandler::handleSigterm);
        setup_unix_signal_handlers();
    }

    void
    UnixSignalHandler::sigintHandler(int)
    {
        char a = 1;
        write(sigintFd[0], &a, sizeof(a));
    }

    void
    UnixSignalHandler::sigtermHandler(int)
    {
        char a = 1;
        write(sigtermFd[0], &a, sizeof(a));
    }

    void
    UnixSignalHandler::handleSigint()
    {
        snInt->setEnabled(false);
        char sint;
        read(sigintFd[1], &sint, sizeof(sint));
        emit stop();
        snInt->setEnabled(true);
    }

    void
    UnixSignalHandler::handleSigterm()
    {
        snTerm->setEnabled(false);
        char sterm;
        read(sigtermFd[1], &sterm, sizeof(sterm));
        emit stop();
        snTerm->setEnabled(true);
    }
}
