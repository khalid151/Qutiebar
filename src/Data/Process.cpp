#include "Data/Process.h"

namespace Data
{
    Process::Process(QProcess *proc)
    {
        this->proc = proc;
        connect(proc, &QProcess::readyReadStandardOutput,
                [this]() { emit update(); });
    }

    void
    Process::enableCustomIcon(Widgets::Icon *icon)
    {
        if(icon == nullptr) return;
        customIcon = true;
        this->icon = icon;
    }

    // Private
    float
    Process::getData()
    {
        if(proc->isOpen())
        {
            QString recv = proc->readAll();
            for(const auto &d:recv.split('\n'))
                if(d.startsWith('v'))
                    data = d.section(":", 1, 1).toFloat();
                else if(d.startsWith('m'))
                    max = d.section(":", 1, 1).toFloat();
                else if(d.startsWith('u'))
                    unit = d.section(":", 1, 1);
                else if(d.startsWith('i') && customIcon)
                    icon->load(d.section(":", 1, 1));
        }

        if(data > max) max = data; // Update max value if not specified.
        return data;
    }
}
