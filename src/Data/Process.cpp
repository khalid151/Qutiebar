#include "Data/Process.h"

namespace Data
{
    Process::Process(QProcess *proc)
    {
        this->proc = proc;
        QProcess::connect(proc, &QProcess::readyReadStandardOutput,
                [this]() { emit update(); });
    }

    int
    Process::getData()
    {
        if(proc->isOpen())
            data = proc->readAll().simplified().toInt();
        if(data > max) max = data; // Update max value if not specified.
        return data;
    }
}
