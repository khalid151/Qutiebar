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

        private:
            int data = 0;
            QProcess *proc;
            int getData();
            void setData();
    };
}

#endif // PROCESSDATA_H
