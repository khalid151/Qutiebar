#ifndef BACKLIGHTDATA_H
#define BACKLIGHTDATA_H

#include "Utils/Misc.h"

namespace Data
{
    class Backlight : public Utils::DataModel
    {
        public:
            Backlight(const QString&, int = 1000);

            inline int getMax() { return maxBrightness; }

        private:
            QString backlight;
            int maxBrightness = 100;
            int getData();
            QString getName() { return "Backlight"; }
            QString getUnit() { return "%"; }
    };
}

#endif // BACKLIGHTDATA_H
