#ifndef CLOCKWID_H
#define CLOCKWID_H

#include <QHBoxLayout>

#include "Widgets/Text.h"

namespace Modules
{
    class Clock : public Widgets::Text
    {
        public:
            Clock(int p = 0, const QString& = "hh:mm A", int = 1000, QWidget* = nullptr);

            void updateFormat(const QString&);
            void setAltFormat(const QString&);

        private:
            QString format, altFormat;
            QHBoxLayout layoutContainer;

        private slots:
            void switchFormat();
            void updateClock();
    };
}

#endif // CLOCKWID_H
