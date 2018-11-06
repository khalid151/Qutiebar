#ifndef PROCESSDATA_H
#define PROCESSDATA_H

#include <QProcess>

#include "Widgets/Icon.h"
#include "Utils/Misc.h"

namespace Data
{
    class Process : public Utils::DataModel
    {
        public:
            Process(QProcess*);

            void setName(QString n) { name = n; }
            void setMax(int m) { max = m; }
            void setUnit(QString u) { unit = u; }
            void enableCustomIcon(Widgets::Icon*);

        private:
            bool customIcon = false;
            float data = 0;
            float max = 100;
            QString unit = "";
            QString name = "Process";
            QProcess *proc;
            Widgets::Icon *icon = nullptr;

            float getData();
            float getMax() { return max; }
            QString getName() { return name; }
            QString getUnit() { return unit; }
            void setData();
    };
}

#endif // PROCESSDATA_H
