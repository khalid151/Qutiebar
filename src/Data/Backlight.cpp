#include <QFile>
#include <QTextStream>
#include <QTimer>


#include "Data/Backlight.h"

namespace Data
{
    Backlight::Backlight(const QString &backlight, int updateRate)
    {
        this->backlight = backlight;
        QFile file(QString("%1/max_brightness").arg(backlight));
        if(file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&file);
            maxBrightness = in.readAll().trimmed().toInt();
            file.close();
        }

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this](){ emit update(); });
        timer->start(updateRate);
    }

    int
    Backlight::getData()
    {
        QFile file(QString("%1/brightness").arg(backlight));
        if(!file.open(QFile::ReadOnly | QFile::Text))
            return 0;
        QTextStream in(&file);
        QString brightness = in.readAll().trimmed();
        file.close();
        return brightness.toInt();
    }
}
