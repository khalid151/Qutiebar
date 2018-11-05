#include <memory>

#include "Modules/Desktops.h"

namespace Modules
{
    Desktops::Desktops(Type t, Widgets::Icon::IconType ic, const QStringList &desktopIcons, int padding, QWidget *parent)
        : QWidget(parent), Utils::WidgetProperties(this)
    {
        this->padding = padding;
        setLayout(&layoutContainer);
        layoutContainer.setContentsMargins(0, 0, 0, 0);
        type = t;
        // If it's a text, defaultIcon should be empty, else, use resources.
        ic == Widgets::Icon::TEXT ? defaultIcon = "" : defaultIcon = ":empty.svg";
        iconType = ic;
        widgetsCount = 0;
        icons = desktopIcons;
        bool createWidgets;
        if(!icons.isEmpty() && (createWidgets = true))
            for(const auto &i:icons)
            {
                if(i.section(":", 0, 0).contains("active"))
                {
                    activeIconType = true;
                    createWidgets = false;
                    if(i.section(":", 0, 0).simplified() == "active")
                        activeIcon = i.section(":", 1, 1).simplified();
                    else
                        defaultIcon = i.section(":", 1, 1).simplified();
                }

                if(createWidgets)
                {
                    Widgets::Icon *wid;
                    if(ic != Widgets::Icon::TEXT)
                    {
                        wid = new Widgets::Icon(i.section(":", 1, 1).simplified(), h, w, padding, this);
                        wid->setAntialiasing(antialiasing, smoothing);
                    }
                    else
                        wid = new Widgets::Icon(i.section(":", 1, 1).simplified(), pt, padding, this);
                    layoutContainer.addWidget(wid);
                    widgetsCount++;

                    // set default icon if the list has it
                    if(i.section(":", 0, 0).simplified() == "default")
                        defaultIcon = i.section(":", 1, 1).simplified();
                }
            }
        else
        {
            defaultLayout = true;
            iconType = Widgets::Icon::TEXT;
        }
        setIndicatorStyle(Indicator::ACTIVE, Qt::white, Qt::transparent, Qt::red, 2, Lines::UNDERLINE);
    }

    void
    Desktops::setSpacing(int s)
    {
        layoutContainer.setSpacing(s);
    }

    void
    Desktops::setAntialiasing(bool aa, bool smoothing)
    {
        antialiasing = aa;
        this->smoothing = smoothing;
        for(int i = 0; i < layoutContainer.count(); i++)
        {
            auto *item = dynamic_cast<Widgets::Icon*>(layoutContainer.itemAt(i)->widget());
            item->setAntialiasing(aa, smoothing);
        }
    }

    void
    Desktops::resize(int pt)
    {
        this->pt = pt;
        for(int i = 0; i < layoutContainer.count(); i++)
        {
            auto *item = dynamic_cast<Widgets::Icon*>(layoutContainer.itemAt(i)->widget());
            item->resize(pt);
        }
    }

    void
    Desktops::resize(int h, int w)
    {
        this->h = h;
        this->w = w;
        for(int i = 0; i < layoutContainer.count(); i++)
        {
            auto *item = dynamic_cast<Widgets::Icon*>(layoutContainer.itemAt(i)->widget());
            item->resize(h, w);
        }
    }

    void
    Desktops::setVerticalAlignment(int va)
    {
        this->va = va;
        for(int i = 0; i < layoutContainer.count(); i++)
        {
            auto *item = dynamic_cast<Widgets::Icon*>(layoutContainer.itemAt(i)->widget());
            item->setVerticalAlignment(va);
        }
    }

    void
    Desktops::setIndicatorStyle(Indicator i, const QColor &foreground,
            const QColor &background, const QColor &lineColor, int lineWidth, Lines line)
    {
        auto indicator = std::make_unique<QWidget>();
        auto props = std::make_unique<Utils::WidgetProperties>(indicator.get());

        props->setForeground(foreground);
        props->setBackground(background);

        if(line == Lines::UNDERLINE) props->setUnderline(lineWidth, lineColor);
        if(line == Lines::OVERLINE) props->setOverline(lineWidth, lineColor);

        if(i == Indicator::ACTIVE)
            indicatorStyle = indicator->styleSheet();
        else
            inactiveStyle = indicator->styleSheet();
    }

    void
    Desktops::updateDesktops(const QStringList &desktops, int index)
    {
        currentIndex = index;
        if(desktops != currentDesktops)
        {
            if(widgetsCount < desktops.size())
                for(const auto &x:desktops)
                    if(!currentDesktops.contains(x))
                    {
                        Widgets::Icon *td;
                        if(iconType != Widgets::Icon::TEXT)
                        {
                            td = new Widgets::Icon(defaultIcon, h, w, padding, this);
                            td->setVerticalAlignment(va);
                            td->setAntialiasing(antialiasing, smoothing);
                        }
                        else
                        {
                            td = new Widgets::Icon(defaultIcon, pt, padding, this);
                            td->setFont(this->font());
                            td->setVerticalAlignment(va);
                        }
                        layoutContainer.addWidget(td);
                        widgetsCount++;
                    }
            currentDesktops = desktops;
        }
        // Hide unpopulated desktops
        for(int i = 0; i < layoutContainer.count(); i++)
            layoutContainer.itemAt(i)->widget()->setHidden(true);

        // Displaying populated desktops:
        switch(type)
        {
            case CURRENT:
                for(int i = 0; i < currentDesktops.size(); i++)
                {
                    if(i == currentIndex)
                    {
                        auto *item = dynamic_cast<Widgets::Icon*>(layoutContainer.itemAt(i)->widget());
                        if(defaultLayout)
                            item->load(currentDesktops[i]);
                        else
                            for(const auto &ic:icons)
                            {
                                if((i + 1) == ic.section(":", 0, 0).toInt())
                                    item->load(ic.section(":", 1).simplified());
                            }
                        item->setHidden(false);
                    }
                }
                break;

            case POPULATED:
                for(int i = 0; i < currentDesktops.size(); i++)
                {
                    auto *item = dynamic_cast<Widgets::Icon*>(layoutContainer.itemAt(i)->widget());


                    if(i == index) item->setStyleSheet(indicatorStyle);
                    else item->setStyleSheet(inactiveStyle);

                    if(activeIconType)
                    {
                        if(i == index) item->load(activeIcon);
                        else item->load(defaultIcon);
                    }
                    else if(defaultLayout && iconType == Widgets::Icon::TEXT)
                    {
                        item->load(currentDesktops[i]);
                    }
                    else
                    {
                        // Load default icon before checking for the icon list
                        if(!defaultIcon.isEmpty()) item->load(defaultIcon);
                        else item->load(currentDesktops[i]);
                        for(const auto &ic:icons)
                        {
                            if(currentDesktops[i] == ic.section(":", 0, 0).simplified())
                                item->load(ic.section(":", 1).simplified());
                        }
                    }
                    item->setHidden(false);
                }
                break;
        }
    }
}
