#include <functional>
#include <QTimer>

#include "Modules/MusicDisplay.h"

namespace Modules
{
    MusicDisplay::MusicDisplay(Utils::MusicAdaptor *m, int updateRate, QWidget *parent)
        : QWidget(parent), Utils::WidgetProperties(this)
    {
        setLayout(&layoutContainer);
        layoutContainer.setContentsMargins(padding, 0, padding, 0);

        adaptor = m;

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MusicDisplay::updateWidget);
        timer->start(updateRate);
        updateWidget();
    }

    void
    MusicDisplay::addProgress(int length, int width, int valign, const QColor &color)
    {
        showProgress = true;
        _progress = std::make_unique<Widgets::Progress>(length, 0, this);
        _progress->setStyle(width, color, Qt::black);
        _progress->setVerticalAlignment(valign);
        layoutContainer.addWidget(_progress.get());
    }

    void
    MusicDisplay::addText(const QString& f, int width, float speed, int padding, bool rt)
    {
        showText = true;
        displayText = std::make_unique<Widgets::Text>("", padding, this);
        if(rt) displayText->enableRichText();
        layoutContainer.addWidget(displayText.get());
        if(width != 0)
        {
            displayText->setMaxLength(width, speed);
        }
        adaptor->setSongInfoFormat(f);
    }

    void
    MusicDisplay::addButtons(int s, const QStringList &l)
    {
        addButtons(Widgets::IconType::Text, false, s, 0, l);
    }

    void
    MusicDisplay::addButtons(int w, int h, bool aa, const QStringList &l)
    {
        addButtons(Widgets::IconType::Pixmap, aa, w, h, l);
    }

    // Private
    void
    MusicDisplay::addButtons(Widgets::IconType::IconType t, bool aa, int width, int height, const QStringList &l)
    {
        using namespace Widgets;
        using namespace Utils;
        _buttons = std::make_unique<QWidget>(this);
        buttonsContainer = std::make_unique<QHBoxLayout>(_buttons.get());
        buttonsContainer->setContentsMargins(0, 0, 0, 0);
        layoutContainer.addWidget(_buttons.get());
        for(const auto &b:l)
        {
            auto button = b.section(":", 0, 0).simplified();
            auto icon = b.section(":", 1, 1).simplified();

            Icon *w;
            if(t == IconType::Text)
                w = new Icon(icon, width, 0, _buttons.get());
            else
                w = new Icon(icon, width, height, 0, _buttons.get());

            if(t == IconType::Pixmap)
                w->setAntialiasing(aa, aa);
            buttonsContainer->addWidget(w);

            if(!QString::compare(button, "play", Qt::CaseInsensitive))
                connect(w->event, &EventHandler::MouseClick, this, [this](){ adaptor->play(); });
            if(!QString::compare(button, "pause", Qt::CaseInsensitive))
                connect(w->event, &EventHandler::MouseClick, this, [this](){ adaptor->pause(); });
            if(!QString::compare(button, "stop", Qt::CaseInsensitive))
                connect(w->event, &EventHandler::MouseClick, this, [this](){ adaptor->stop(); });
            if(!QString::compare(button, "next", Qt::CaseInsensitive))
                connect(w->event, &EventHandler::MouseClick, this, [this](){ adaptor->next(); });
            if(!QString::compare(button, "previous", Qt::CaseInsensitive))
                connect(w->event, &EventHandler::MouseClick, this, [this](){ adaptor->previous(); });
            if(!QString::compare(button, "toggle", Qt::CaseInsensitive))
            {
                connect(w->event, &EventHandler::MouseClick, this, [this](){ adaptor->playPause(); });;
                auto pausedIcon = b.section(":", 2, 2).simplified();
                if(!pausedIcon.isEmpty())
                {
                    auto iconSwitch = [this](auto p, auto s)
                    {
                        if(adaptor->getState() == Utils::MusicAdaptor::PLAYING)
                            return p;
                        else
                            return s;
                    };
                    auto loadIcon = std::bind(&Icon::load, w, std::bind(iconSwitch, pausedIcon, icon));
                    loadIcon();
                    connect(w->event, &EventHandler::MouseClick, w, loadIcon);
                }
            }
        }
    }

    void
    MusicDisplay::updateWidget()
    {
        if(showProgress)
        {
            float pos = adaptor->getSongTimePercentage();
            _progress->updateProgress(static_cast<int>(pos * 100));
        }

        if(showText && adaptor->getState() != Utils::MusicAdaptor::PAUSED)
        {
            auto currentInfo = adaptor->getSongInfo();
            if(adaptor->getState() == Utils::MusicAdaptor::STOPPED)
                currentInfo = "";
            if(oldInfo != currentInfo )
            {
                oldInfo = currentInfo;
                displayText->updateText(currentInfo);
            }
        }
    }
}
