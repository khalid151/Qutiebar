#include "Utils/EventHandler.h"

namespace Utils
{
    EventHandler::EventHandler(QObject *parent) : QObject(parent)
    {
    }

    bool
    EventHandler::eventFilter(QObject *obj, QEvent *event)
    {
        if (event->type() == QEvent::HoverEnter) {
            emit MouseEnter();
            return true;
        } else if (event->type() == QEvent::HoverLeave) {
            emit MouseLeave();
            return true;
        } else if (event->type() == QEvent::MouseButtonPress) {
            emit MouseClick();
            return true;
        }

        return QObject::eventFilter(obj, event);
    }
}
