#ifndef DESKTOPS_H
#define DESKTOPS_H

#include "Widgets/Icon.h"
#include "Widgets/Text.h"
#include "Utils/WidgetProperties.h"

namespace Modules
{
    class Desktops : public QWidget, public Utils::WidgetProperties
    {
        public:
            enum Type{CURRENT, POPULATED};
            enum Lines{NONE = 0, UNDERLINE = 1 << 0, OVERLINE = 1 << 1};
            enum Indicator{ACTIVE, INACTIVE};

            Desktops(Type, Widgets::Icon::IconType, const QStringList&, int = 0, QWidget* = nullptr);

            void setSpacing(int);
            void setAntialiasing(bool, bool = false);
            void resize(int);
            void resize(int, int);
            void setVerticalAlignment(int);
            void setIndicatorStyle(Indicator, const QColor&, const QColor&, const QColor&, int, Lines);

        private:
            bool activeIconType = false; // When only active and inactive icons are set: - x - - (where: x is active, - is inactive)
            bool defaultLayout = false;
            bool antialiasing = true, smoothing = true;
            Widgets::Icon::IconType iconType;
            unsigned char type;
            int currentIndex, widgetsCount, padding;
            int h = 25, w = 25, pt = 10, va = 0;
            QStringList currentDesktops, icons;
            QString activeIcon, defaultIcon;
            QString indicatorStyle, inactiveStyle = "";
            QHBoxLayout layoutContainer;

        public slots:
            void updateDesktops(const QStringList&, int);
    };
}

#endif // DESKTOPS_H
