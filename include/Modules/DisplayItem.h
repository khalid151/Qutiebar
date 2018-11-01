#ifndef DISPLAYITEM_H
#define DISPLAYITEM_H

#include "Widgets/Icon.h"
#include "Widgets/Progress.h"
#include "Widgets/Text.h"
#include "Utils/EventHandler.h"
#include "Utils/Misc.h" // For DataModel base class
#include "Utils/WidgetProperties.h"

namespace Modules
{
    class DisplayItem : public QWidget, public Utils::WidgetProperties
    {
        public:
            enum Type{TEXT, FONTICON, PIXICON, CIRCLE, TEXTCIRCLE};
            enum TextLocation{LEFT,RIGHT,NONE};

            DisplayItem(Type = Type::TEXT, TextLocation = TextLocation::NONE,
                    int w = 20, int h = 20, int padding = 0,
                    int updateRate = 1000, QWidget* parent = nullptr);
            ~DisplayItem();

            void setIcons(const QStringList&);
            void setIcons(const QStringList&, const QStringList&);
            void setColor(const QColor&);
            void setColors(const QColor&, const QColor&);
            void enableIconColorChange(bool = true);
            void setData(Utils::DataModel*);

            Widgets::Text *percentage = nullptr;
            Widgets::Icon *icon = nullptr;
            Widgets::Progress *progress = nullptr;
            Utils::EventHandler *event;

        private:
            bool modelHasState = false;
            bool staticIconColor = true;
            char displayType, textLocation;
            int max = 100;
            QStringList primaryIcons, secondaryIcons;
            QColor primaryColor, secondaryColor;
            QHBoxLayout layoutContainer;
            Utils::DataModel *M = nullptr;

            void updateItem();
            int getPercentage();
            QString getCurrentIcon(const QStringList&);
    };
}

#endif // DISPLAYITEM_H
