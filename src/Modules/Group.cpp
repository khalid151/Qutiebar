#include "Modules/Group.h"

namespace Modules
{
    Group::Group(int padding, QWidget *parent)
        : QWidget(parent), Utils::WidgetProperties(this, false)
    {
        event = new Utils::EventHandler;
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        this->padding = padding;

        setLayout(&layoutContainer);
        layoutContainer.setSpacing(0);
        layoutContainer.setContentsMargins(padding, 0, padding, 0);
    }

    Group::~Group()
    {
        delete event;
    }

    void
    Group::addWidget(QWidget *widget)
    {
        layoutContainer.addWidget(widget);
    }

    void
    Group::setSpacing(int spacing)
    {
        layoutContainer.setSpacing(spacing);
    }

    void
    Group::setMargins(int m)
    {
        layoutContainer.setContentsMargins(padding + m, m, padding + m, m);
    }
}
