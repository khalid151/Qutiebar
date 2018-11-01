#include "Utils/TextScroller.h"
#include "Utils/Misc.h"

namespace Utils
{
    Scroller::Scroller(bool rt, QObject *parent)
        : QThread(parent)
    {
        richText = rt;
    }

    void
    Scroller::setText(const QString& text, const int maxLength)
    {
        shouldStop = false;
        this->text = text;
        this->maxLength = maxLength;
    }

    void
    Scroller::setScrollSpeed(const float secs)
    {
        pause = static_cast<int>(secs * 1000);
    }

    void
    Scroller::stop()
    {
        shouldStop = true;
        quit();
        wait();
    }

    // Private
    void
    Scroller::run()
    {
        auto section = [this](auto t, auto s, auto e) {
            if(richText) return Utils::section(t, s, e);
            else return t.section("", s, e);
        };
        auto len = [this](auto t) {
            if(richText) return Utils::rawStrLength(t);
            else return t.count();
        };
        text = text.append(" - ");
        emit sendText(section(text, 1, maxLength));
        msleep(pause);
        forever
        {
            if(shouldStop)
                break;

            for(int i = 0; i < len(text); i ++)
            {
                if(shouldStop) break;
                emit sendText(section(text, i + 1, i + maxLength));
                if(i > len(text) - maxLength)
                    emit sendText(section(text, i + 1, len(text)).append(
                                section(text, 1, maxLength - (len(text) - i))));
                msleep(pause);
            }
        }
    }
}
