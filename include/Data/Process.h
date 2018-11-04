#ifndef PROCESSDATA_H
#define PROCESSDATA_H

#include <QProcess>

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

        private:
            int data = 0;
            int max = 100;
            QString unit = "";
            QString name = "Process";
            QProcess *proc;

            int getData();
            int getMax() { return max; }
            QString getName() { return name; }
            QString getUnit() { return unit; }
            void setData();
    };
}

#endif // PROCESSDATA_H
