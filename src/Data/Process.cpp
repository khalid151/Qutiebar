#include <functional>

#include "Data/Process.h"

namespace Data
{
    Process::Process(QProcess *proc)
    {
        this->proc = proc;
        QProcess::connect(proc, &QProcess::readyReadStandardOutput,
                std::bind(&Process::setData, this));
    }

    int
    Process::getData()
    {
        return data;
    }

    void
    Process::setData()
    {
        if(proc->isOpen())
            data = proc->readAll().simplified().toInt();
    }
}
