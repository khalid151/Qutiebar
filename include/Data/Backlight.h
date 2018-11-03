#ifndef BACKLIGHTDATA_H
#define BACKLIGHTDATA_H

#include "Utils/Misc.h"

namespace Data
{
    class Backlight : public Utils::DataModel
    {
        public:
            Backlight(const QString&);

            inline int getMax() { return maxBrightness; }

        private:
            QString backlight;
            int maxBrightness = 100;
            int getData();
    };
}

#endif // BACKLIGHTDATA_H
