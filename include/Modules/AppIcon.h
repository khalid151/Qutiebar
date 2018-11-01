#ifndef APPICON_H
#define APPICON_H

#include "Widgets/Icon.h"
#include "Utils/X11EventHandler.h"

namespace Modules
{
    class AppIcon : public Widgets::Icon
    {
        public:
            AppIcon(Utils::X11EventHandler*, int, int, int, QWidget* = nullptr);

            inline void setDefaultIcon(const QString &icon)
            { defaultIcon = icon; }

        private:
            Utils::X11EventHandler *x_handler;
            QString defaultIcon = ":empty.svg"; // From resources

            void updateIcon(xcb_window_t);
    };
}

#endif // APPICON_H
