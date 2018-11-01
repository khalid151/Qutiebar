#ifndef PROGRESS_H
#define PROGRESS_H

#include <QColor>
#include <QFont>
#include <QPaintEvent>
#include <QPen>
#include <QWidget>

#include "Utils/EventHandler.h"
#include "Utils/WidgetProperties.h"

namespace Widgets
{
    class Progress : public QWidget, public Utils::WidgetProperties
    {
        public:
            Progress(int, bool, int = 0, QWidget* = nullptr); // Circle
            Progress(int, int = 0, QWidget* = nullptr); // Line

            void setColor(const QColor&);
            void setFontColor(const QColor&);
            void setStyle(int, const QColor&, const QColor&, const QColor&, int = 0);
            void setStyle(int, const QColor&, const QColor&);
            void setVerticalAlignment(int);
            void updateProgress(int);
            void setCircleIcon(const QString&);
            void resize(int, int);
            Utils::EventHandler *event;

        protected:
            void paintEvent(QPaintEvent*);

        private:
            bool showPercent, showIcon = false;
            char type;
            int progress, shapeHeight, shapeWidth, penWidth, secondPenWidth;
            int valign = 0;
            QColor primaryColor, secondaryColor, fontColor;
            QString icon;
    };
}

#endif // PROGRESS_H
