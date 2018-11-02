#include <QTimer>

#include "Modules/DisplayItem.h"

namespace Modules
{
    DisplayItem::DisplayItem(Type display, TextLocation location, int w, int h,
            int padding, int updateRate, QWidget *parent)
        : QWidget(parent), Utils::WidgetProperties(this)
    {
        event = new Utils::EventHandler;
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        setLayout(&layoutContainer);
        this->padding = padding;
        layoutContainer.setContentsMargins(padding, 0, padding, 0);
        layoutContainer.setSpacing(0);
        displayType = display;
        textLocation = location;

        switch(display)
        {
            case Type::TEXT:
                _percentage = std::make_unique<Widgets::Text>("", padding, this);
                _percentage->setStyleSheet("background: none; border: none");
                layoutContainer.addWidget(_percentage.get());
                break;
            case Type::FONTICON:
                _icon = std::make_unique<Widgets::Icon>("", 10, padding, this);
                break;
            case Type::PIXICON:
                _icon = std::make_unique<Widgets::Icon>("", h, w, padding, this);
                break;
            case Type::CIRCLE:
                _progress = std::make_unique<Widgets::Progress>(w, false, 0, this);
                layoutContainer.addWidget(_progress.get());
                break;
            case Type::TEXTCIRCLE:
                _progress = std::make_unique<Widgets::Progress>(w, true, 0, this);
                layoutContainer.addWidget(_progress.get());
                break;
        }

        // Text location relative to icon
        if(location != TextLocation::NONE)
        {
            _percentage = std::make_unique<Widgets::Text>("", padding, this);
            _percentage->setStyleSheet("background: none; border: none");
            switch(location)
            {
                case TextLocation::LEFT:
                    layoutContainer.addWidget(_percentage.get());
                    layoutContainer.addWidget(_icon.get());
                    break;
                case TextLocation::RIGHT:
                    layoutContainer.addWidget(_icon.get());
                    layoutContainer.addWidget(_percentage.get());
                    break;
                case TextLocation::NONE:
                    break;
            }
        } else if(_icon != nullptr)
        {
            layoutContainer.addWidget(_icon.get());
        }

        primaryColor = secondaryColor = Qt::black;

        if(_progress != nullptr)
            _progress->setStyle(4, Qt::white, Qt::black, Qt::white, 2);

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &DisplayItem::updateItem);
        timer->start(updateRate);
        updateItem();
    }

    void
    DisplayItem::setIcons(const QStringList &iconsList)
    {
        primaryIcons = iconsList;
    }

    void
    DisplayItem::setIcons(const QStringList &list1, const QStringList &list2)
    {
        primaryIcons = list1;
        secondaryIcons = list2;
    }

    void
    DisplayItem::setColor(const QColor &c)
    {
        primaryColor = c;
    }

    void
    DisplayItem::setColors(const QColor &c1, const QColor &c2)
    {
        primaryColor = c1;
        secondaryColor = c2;
    }

    void
    DisplayItem::enableIconColorChange(bool enable)
    {
        staticIconColor = !enable;
    }

    void
    DisplayItem::setData(Utils::DataModel *m)
    {
        M = m;
        max = M->getMax();
        modelHasState = M->hasState();
        updateItem();
    }

    // Private
    void
    DisplayItem::updateItem()
    {
        switch(displayType)
        {
            case Type::CIRCLE:
            case Type::TEXTCIRCLE:
                _progress->updateProgress(getPercentage());
                _progress->update();
                if(modelHasState)
                {
                    if(M->getState()) _progress->setColor(primaryColor);
                    else _progress->setColor(secondaryColor);
                }
                break;
            case Type::TEXT:
                _percentage->setText(QString("%1%").arg(QString::number(getPercentage())));
                if(modelHasState)
                {
                    if(M->getState()) _percentage->setForeground(primaryColor);
                    else _percentage->setForeground(secondaryColor);
                }
                break;
            case Type::FONTICON:
            case Type::PIXICON:
                if(_percentage !=nullptr)
                    _percentage->setText(QString("%1%").arg(QString::number(getPercentage())));
                if(modelHasState)
                {
                    if(!primaryIcons.isEmpty() && !secondaryIcons.isEmpty())
                    {
                        if(M->getState())
                            _icon->load(getCurrentIcon(primaryIcons));
                        else if(!M->getState())
                            _icon->load(getCurrentIcon(secondaryIcons));
                        else if(M->getState() == 2)
                            _icon->load(primaryIcons.at(primaryIcons.count() - 1)); // Mostly for battery, last icon would represent full.

                        if(!staticIconColor)
                        {
                            if(M->getState()) _icon->setForeground(primaryColor);
                            else _icon->setForeground(secondaryColor);
                        }
                    }
                }
                else
                {
                    if(!primaryIcons.isEmpty())
                        _icon->load(getCurrentIcon(primaryIcons));
                }
                break;
        }
    }

    int
    DisplayItem::getPercentage()
    {
        if(M != nullptr)
            return static_cast<int>(100 * static_cast<float>(M->getData())/max);
        else
            return 0;
    }

    QString
    DisplayItem::getCurrentIcon(const QStringList &l)
    {
       int index = (getPercentage() / (100/l.count()));
       if(index < l.count()) return l.at(index);
       else return l.at(index - 1);
    }

}
