#include <QFile>
#include <QTextStream>

#include "Data/Backlight.h"

namespace Data
{
    Backlight::Backlight(const QString &backlight)
    {
        this->backlight = backlight;
        QFile file(QString("%1/max_brightness").arg(backlight));
        if(file.open(QFile::ReadOnly | QFile::Text))
        {
            QTextStream in(&file);
            maxBrightness = in.readAll().trimmed().toInt();
            file.close();
        }
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
