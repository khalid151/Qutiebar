#include "Modules/DisplayItem.h"

namespace Modules
{
    using namespace DisplayType;

    DisplayItem::DisplayItem(Utils::DataModel *m, int padding)
        : Utils::WidgetProperties(this)
    {
        event = new Utils::EventHandler(this);
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        setLayout(&layoutContainer);
        this->padding = padding;
        layoutContainer.setContentsMargins(padding, 0, padding, 0);
        layoutContainer.setSpacing(0);

        M = m;
        unit = M->getUnit();

        connect(M, &Utils::DataModel::update, this, &DisplayItem::update);
    }

    void
    DisplayItem::addIconDisplay(const QString &type, int w, int h, int pt, int p)
    {
        if(type.isEmpty())
            return;
        auto size = font().pointSize();
        if(type == "name") {
            _icon = std::make_unique<Widgets::Icon>(M->getName(), size, p, this);
            name = true;
        }
        else if(type == "fonticon")
            _icon = std::make_unique<Widgets::Icon>(" ", pt, p, this);
        else if(type == "pixmap")
            _icon = std::make_unique<Widgets::Icon>(":empty.svg", w, h, p, this);
        // Create an icon to be set by custom module
        else if(type == "process")
            _icon = std::make_unique<Widgets::Icon>(":empty.svg", w, h, p, this);

        _icon->removeEventFilter(_icon.get()->event);
        layoutContainer.addWidget(icon());
        update();
    }

    void
    DisplayItem::addDataDisplay(int r, int l, int p)
    {
        if(!r && !l)
        {
            dataType = Type::Text;
            _data = std::make_unique<Widgets::Text>("", p, this);
            _data->setFont(font());
            _data->removeEventFilter(dynamic_cast<Widgets::Text*>(_data.get())->event);
        }
        else if(r && !l)
        {
            dataType = Type::Circle;
            _data = std::make_unique<Widgets::Progress>(r, true, p, this);
            _data->removeEventFilter(dynamic_cast<Widgets::Progress*>(_data.get())->event);
        }
        else
        {
            dataType = Type::Line;
            _data = std::make_unique<Widgets::Progress>(l, p, this);
            _data->removeEventFilter(dynamic_cast<Widgets::Progress*>(_data.get())->event);
        }
        layoutContainer.addWidget(data());
        update();
    }

    void
    DisplayItem::setColors(const QColor &c1, const QColor &c2)
    {
        primaryColor = c1;
        secondaryColor = c2;
    }

    void
    DisplayItem::setIcons(const QStringList &pi, const QStringList &si)
    {
        primaryIcons = pi;
        secondaryIcons = si;
    }

    // Private
    void
    DisplayItem::update()
    {
        if(_icon != nullptr && !primaryIcons.isEmpty() && !name)
        {
            if(hasState)
            {
                if(M->getState())
                    _icon.get()->load(getCurrentIcon(primaryIcons));
                else if(!secondaryIcons.isEmpty())
                    _icon.get()->load(getCurrentIcon(secondaryIcons));
            }
            else
                _icon.get()->load(getCurrentIcon(primaryIcons));
        }

        // Update data here
        if(_data != nullptr)
        switch(dataType)
        {
            case Type::Text:
                if(unit == "%")
                    dynamic_cast<Widgets::Text*>(_data.get())->
                    setText(QString::number(getPercent()).append(unit));
                else
                    dynamic_cast<Widgets::Text*>(_data.get())->
                    setText(QString::number(M->getData()).append(unit));
                if(hasState)
                {
                    if(M->getState())
                        dynamic_cast<Widgets::Text*>(_data.get())->setForeground(primaryColor);
                    else
                        dynamic_cast<Widgets::Text*>(_data.get())->setForeground(secondaryColor);
                }
                break;
            case Type::Circle:
            case Type::Line:
                dynamic_cast<Widgets::Progress*>(_data.get())->updateProgress(getPercent());
                if(hasState)
                {
                    if(M->getState())
                        dynamic_cast<Widgets::Progress*>(_data.get())->setColor(primaryColor);
                    else
                        dynamic_cast<Widgets::Progress*>(_data.get())->setColor(secondaryColor);
                }
                break;
        }
    }

    int
    DisplayItem::getPercent()
    {
        return static_cast<int>(100 * static_cast<float>(M->getData())/M->getMax());
    }

    QString
    DisplayItem::getCurrentIcon(const QStringList &l)
    {
       int index = (getPercent() / (100/l.count()));
       if(index < l.count()) return l.at(index);
       else return l.at(index - 1);
    }
}
