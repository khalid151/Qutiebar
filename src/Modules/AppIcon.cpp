#include "Modules/AppIcon.h"

namespace Modules
{
    AppIcon::AppIcon(Utils::X11EventHandler *handler, int w, int h, int p, QWidget *parent)
        : Widgets::Icon(":empty.svg", w, h, p, parent)
    {
        x_handler = handler;
        connect(handler, &Utils::X11EventHandler::winIdChanged, this, &AppIcon::updateIcon);
    }

    // Private
    void
    AppIcon::updateIcon(xcb_window_t win)
    {
        auto icon = x_handler->getIcon(win);
        if(!icon.isNull()) setIcon(std::move(icon));
        else load(defaultIcon);
    }
}
