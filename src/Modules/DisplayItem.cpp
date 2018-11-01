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
                percentage = new Widgets::Text("", padding);
                percentage->setStyleSheet("background: none; border: none");
                layoutContainer.addWidget(percentage);
                break;
            case Type::FONTICON:
                icon = new Widgets::Icon("", 10, padding, this);
                break;
            case Type::PIXICON:
                icon = new Widgets::Icon("", h, w, padding, this);
                break;
            case Type::CIRCLE:
                progress = new Widgets::Progress(w, false, 0, this);
                layoutContainer.addWidget(progress);
                break;
            case Type::TEXTCIRCLE:
                progress = new Widgets::Progress(w, true, 0, this);
                layoutContainer.addWidget(progress);
                break;
        }

        // Text location relative to icon
        if(location != TextLocation::NONE)
        {
            percentage = new Widgets::Text("", padding);
            percentage->setStyleSheet("background: none; border: none");
            switch(location)
            {
                case TextLocation::LEFT:
                    layoutContainer.addWidget(percentage);
                    layoutContainer.addWidget(icon);
                    break;
                case TextLocation::RIGHT:
                    layoutContainer.addWidget(icon);
                    layoutContainer.addWidget(percentage);
                    break;
                case TextLocation::NONE:
                    break;
            }
        } else if(icon != nullptr)
        {
            layoutContainer.addWidget(icon);
        }

        primaryColor = secondaryColor = Qt::black;

        if(progress != nullptr)
            progress->setStyle(4, Qt::white, Qt::black, Qt::white, 2);

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &DisplayItem::updateItem);
        timer->start(updateRate);
        updateItem();
    }

    DisplayItem::~DisplayItem()
    {
        delete percentage;
        delete icon;
        delete event;
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
                progress->updateProgress(getPercentage());
                progress->update();
                if(modelHasState)
                {
                    if(M->getState()) progress->setColor(primaryColor);
                    else progress->setColor(secondaryColor);
                }
                break;
            case Type::TEXT:
                percentage->setText(QString("%1%").arg(QString::number(getPercentage())));
                if(modelHasState)
                {
                    if(M->getState()) percentage->setForeground(primaryColor);
                    else percentage->setForeground(secondaryColor);
                }
                break;
            case Type::FONTICON:
            case Type::PIXICON:
                if(percentage !=nullptr)
                    percentage->setText(QString("%1%").arg(QString::number(getPercentage())));
                if(modelHasState)
                {
                    if(!primaryIcons.isEmpty() && !secondaryIcons.isEmpty())
                    {
                        if(M->getState())
                            icon->load(getCurrentIcon(primaryIcons));
                        else if(!M->getState())
                            icon->load(getCurrentIcon(secondaryIcons));
                        else if(M->getState() == 2)
                            icon->load(primaryIcons.at(primaryIcons.count() - 1)); // Mostly for battery, last icon would represent full.

                        if(!staticIconColor)
                        {
                            if(M->getState()) icon->setForeground(primaryColor);
                            else icon->setForeground(secondaryColor);
                        }
                    }
                }
                else
                {
                    if(!primaryIcons.isEmpty())
                        icon->load(getCurrentIcon(primaryIcons));
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
