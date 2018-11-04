#ifndef BATTERYDATA_H
#define BATTERYDATA_H

#include "Utils/Misc.h"

namespace Data
{
    class Battery : public Utils::DataModel
    {
        public:
            Battery(const QString&, int = 1000);

        private:
            QString battery;
            bool hasState() { return true; }
            int getState();
            int getData();
            QString getName() { return "Battery"; }
            QString getUnit() { return "%"; }
    };
}

#endif // BATTERYDATA_H
