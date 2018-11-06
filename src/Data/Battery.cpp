#include <QFile>
#include <QTextStream>
#include <QTimer>

#include "Data/Battery.h"

namespace Data
{
    Battery::Battery(const QString &battery, int updateRate)
    {
        this->battery = battery;

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this](){ emit update(); });
        timer->start(updateRate);
    }

    int
    Battery::getState()
    {
        QFile file(QString("%1/status").arg(battery));
        if(!file.open(QFile::ReadOnly | QFile::Text))
            return -1;
        QTextStream in(&file);
        QString status = in.readAll().trimmed();

        if (status == "Discharging")
            return 0;
        if ( status == "Charging" )
            return 1;
        if ( status == "Full" )
            return 2;

        return -1;
    }

    float
    Battery::getData()
    {
        QFile file(QString("%1/capacity").arg(battery));
        if(!file.open(QFile::ReadOnly | QFile::Text))
            return 0;
        QTextStream in(&file);
        QString percentage = in.readAll().trimmed();
        file.close();
        return percentage.replace("%", "").toFloat();
    }
}
