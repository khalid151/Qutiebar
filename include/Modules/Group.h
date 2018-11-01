#ifndef GROUPWID_H
#define GROUPWID_H

#include <QHBoxLayout>

#include "Utils/WidgetProperties.h"
#include "Utils/EventHandler.h"

namespace Modules
{
    class Group : public QWidget, public Utils::WidgetProperties
    {
        public:
            Group(int, QWidget* parent = nullptr);
            ~Group();

            void addWidget(QWidget*);
            void setSpacing(int);
            void setMargins(int);
            Utils::EventHandler *event;

        private:
            QHBoxLayout layoutContainer;
    };
}

#endif // GROUPWID_H
