#include <QTimer>
#include <QDateTime>

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
        setText(dateTime.toString(format));
    }
}
