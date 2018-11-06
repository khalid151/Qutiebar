#ifndef BACKLIGHTDATA_H
#define BACKLIGHTDATA_H

#include "Utils/Misc.h"

namespace Data
{
    class Backlight : public Utils::DataModel
    {
        public:
            Backlight(const QString&, int = 1000);

            inline float getMax() { return maxBrightness; }

        private:
            QString backlight;
            float maxBrightness = 100;
            float getData();
            QString getName() { return "Backlight"; }
            QString getUnit() { return "%"; }
    };
}

#endif // BACKLIGHTDATA_H
