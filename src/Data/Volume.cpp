#include <cmath>
#include <QTimer>

#include "Data/Volume.h"

namespace Data
{
    Volume::Volume(const QString &card, const QString &selem_name, int updateRate)
    {
        long min;
        snd_mixer_open(&mixer, 1);
        snd_config_update_free_global();
        snd_mixer_attach(mixer, card.toLatin1().data());
        snd_mixer_selem_register(mixer, nullptr, nullptr);
        snd_mixer_load(mixer);
        snd_mixer_selem_id_alloca(&selem_id);
        snd_mixer_selem_id_set_index(selem_id, 0);
        snd_mixer_selem_id_set_name(selem_id, selem_name.toLatin1().data());
        elem = snd_mixer_find_selem(mixer, selem_id);
        snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this](){ emit update(); });
        timer->start(updateRate);
    }

    Volume::~Volume()
    {
        snd_mixer_close(mixer);
    }

    float
    Volume::getData()
    {
        snd_mixer_handle_events(mixer); // To update percentage

        int channelCount = 0;
        long volume, totalVolume = 0;
        for(int i = 0; i <= SND_MIXER_SCHN_LAST; i++)
            if(snd_mixer_selem_has_playback_channel(elem, static_cast<snd_mixer_selem_channel_id_t>(i)))
            {
                snd_mixer_selem_get_playback_volume(elem, static_cast<snd_mixer_selem_channel_id_t>(i), &volume);
                totalVolume += volume;
                channelCount++;
            }
        long finalVolume = std::lround(static_cast<float>(totalVolume/channelCount)/max*100);
        return finalVolume;
    }
}
