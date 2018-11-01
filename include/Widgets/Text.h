#ifndef TEXT_H
#define TEXT_H

#include <QLabel>

#include "Utils/EventHandler.h"
#include "Utils/TextScroller.h"
#include "Utils/WidgetProperties.h"

namespace Widgets
{
    class Text : public QLabel, public Utils::WidgetProperties
    {
        public:
            Text(const QString&, int = 0, QWidget *parent = nullptr);
            ~Text();

            void enableRichText();
            void setMaxLength(const int, const float = 0.25);
            void updateText(const QString&); // To check for text length before setting it (if enabled)

            Utils::EventHandler *event;

        private:
            bool maxLengthSet = false;
            int maxLength;
            Utils::Scroller *textScroller = nullptr;
    };
}

#endif // TEXT_H
