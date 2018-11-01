#ifndef ICON_H
#define ICON_H

#include <memory>
#include <QHBoxLayout>
#include <QIcon>

#include "Widgets/Text.h"

namespace Widgets
{
    class PixIcon : public QWidget
    {
        public:
            PixIcon(int h = 25, int w = 25, const QString &l = "", QWidget* = nullptr);

            void setAA(bool); // Antialiasing
            void setSmoothing(bool);
            void load(const QString&);
            void load(const QIcon&);
            void resize(int, int);

        protected:
            bool loadPixmap = true;
            QIcon qIcon; // Store it just in case resize is needed
            QPixmap icon;
            QString storedIcon;
            void paintEvent(QPaintEvent*);

        private:
            bool aa = true, smooth = false;
            int width, height;
    };

    class Icon : public QWidget, public Utils::WidgetProperties
    {
        public:
            enum IconType {PIXMAP, TEXT};
            Icon(const QString&, int w, int h, int , QWidget* = nullptr); // For pixmap icon
            Icon(const QString&, int ps, int , QWidget* = nullptr); // For font icon
            ~Icon();

            void setAntialiasing(bool, bool = false);
            void load(const QString&);
            void setIcon(const QIcon&); // For pixmap
            void resize(int); // For font size
            void resize(int, int); // For pixmap

            Utils::EventHandler *event;

        private:
            char type; // Pixmap\Text
            std::unique_ptr<QWidget> icon;
            QHBoxLayout layoutContainer;
    };
}

#endif // ICON_H
