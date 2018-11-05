#ifndef DISPLAYITEM_H
#define DISPLAYITEM_H

#include <memory>

#include "Widgets/Icon.h"
#include "Widgets/Progress.h"
#include "Widgets/Text.h"
#include "Utils/EventHandler.h"
#include "Utils/Misc.h" // For DataModel base class
#include "Utils/WidgetProperties.h"

namespace Modules
{
    namespace DisplayType
    {
        enum Type{Text, Circle, Line};
    }

    class DisplayItem : public QWidget, public Utils::WidgetProperties
    {
        public:
            DisplayItem(Utils::DataModel*, int = 0);

            void addIconDisplay(const QString&, int, int, int, int = 0);
            void addDataDisplay(int = 0, int = 0, int = 0);

            void setColors(const QColor&, const QColor&);
            void setIcons(const QStringList&, const QStringList& = {});

            // To style them in Builder
            Widgets::Icon *icon() { return _icon.get(); }
            QWidget *data() { return _data.get(); }

            Utils::EventHandler *event;

        private:
            bool hasState = false, name = false;
            DisplayType::Type dataType;
            QColor primaryColor = Qt::black, secondaryColor = Qt::white;
            QHBoxLayout layoutContainer;
            QString unit = "";
            QStringList primaryIcons, secondaryIcons;
            Utils::DataModel *M = nullptr;
            std::unique_ptr<Widgets::Icon> _icon;
            std::unique_ptr<QWidget> _data;

            void update();
            int getPercent();
            QString getCurrentIcon(const QStringList&);
    };
}

#endif // DISPLAYITEM_H
