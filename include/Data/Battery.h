#ifndef BATTERYDATA_H
#define BATTERYDATA_H

#include "Utils/Misc.h"

namespace Data
{
    class Battery : public Utils::DataModel
    {
        public:
            Battery(const QString&);

        private:
            QString battery;
            bool hasState() { return true; }
            int getState();
            int getData();
    };
}

#endif // BATTERYDATA_H
