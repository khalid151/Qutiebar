#include <cstdio>
#include <QDateTime>
#include <QTimer>

#include "Modules/Clock.h"

namespace Modules
{
    Clock::Clock(int padding, const QString &format, int updateRate, QWidget *parent)
        : Widgets::Text(format, padding, parent)
    {
        this->format = format;
        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Clock::updateClock);
        connect(this->event, &Utils::EventHandler::MouseClick, this,
                &Clock::switchFormat);
        timer->start(updateRate);
        updateClock();
    }

    void
    Clock::updateFormat(const QString &format)
    {
        this->format = format;
    }

    void
    Clock::setAltFormat(const QString &format)
    {
        altFormat = format;
    }

    void
    Clock::setTimeZone(const QString &zone)
    {
        tz = QTimeZone(zone.toLatin1());
        if(!tz.isValid())
        {
            fprintf(stderr, "%s is an invalid time-zone format.\n", zone.toLatin1().data());
            exit(1);
        }
    }

    // Private
    void
    Clock::switchFormat()
    {
        format.swap(altFormat);
    }

    void
    Clock::updateClock()
    {
        QDateTime dateTime = QDateTime::currentDateTime();
        if(tz.isValid())
            setText(dateTime.toTimeZone(tz).toString(format));
        else
            setText(dateTime.toString(format));
    }
}
