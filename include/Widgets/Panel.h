#ifndef PANEL_H
#define PANEL_H

#include <QHBoxLayout>
#include <QRect>
#include <QWidget>

#include "Utils/EventHandler.h"
#include "Utils/WidgetProperties.h"

namespace Widgets
{
    class Panel : public QWidget, public Utils::WidgetProperties
    {
        public:
            Panel(const QRect&, int, int, int margin = 0, int padding = 0);

            void setBackground(const QColor&);
            void setTexture(const QString&);
            void setBorder(int, const QColor&);
            void setRoundCorners(int radius = 10);
            void addModule(const char, QWidget*);
            void setSpacing(const char, int);
            void setPadding(int);
            void setMargins(int);

            Utils::EventHandler *event = nullptr;

        private:
            QWidget *mainWidget = nullptr; // Will hold layouts + widgets
            QWidget *background = nullptr; // Only for color so child widgets won't be affected
            QHBoxLayout mainLayout, leftLayout, centerLayout, rightLayout;
            int screenHeight, screenWidth, panelHeight, panelWidth;
    };
}

#endif // PANEL_H
