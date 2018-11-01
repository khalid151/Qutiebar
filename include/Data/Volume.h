#ifndef VOLUMEDATA_H
#define VOLUMEDATA_H

#include <alsa/asoundlib.h>

#include "Modules/DisplayItem.h"
#include "Utils/Misc.h"

namespace Data
{
    class Volume : public Utils::DataModel
    {
        public:
            Volume(const QString&, const QString&);
            ~Volume();

        private:
            snd_mixer_t *mixer{};
            snd_mixer_selem_id_t *selem_id{};
            snd_mixer_elem_t *elem;
            long max{};
            int getData(); // should return volume here
    };
}

#endif // VOLUMEDATA_H