#ifndef MUSICDISPLAY_H
#define MUSICDISPLAY_H

#include <memory>
#include <QHBoxLayout>

#include "Widgets/Icon.h"
#include "Widgets/Progress.h"
#include "Widgets/Text.h"
#include "Utils/EventHandler.h"
#include "Utils/Misc.h"
#include "Utils/WidgetProperties.h"

namespace Modules
{
    class MusicDisplay : public QWidget, public Utils::WidgetProperties
    {
        public:
            MusicDisplay(Utils::MusicAdaptor*, int = 1000, QWidget* = nullptr);

            Widgets::Progress* getProgress(); // So it would be styled later
            void addProgress(int = 100, int = 2, int = 0, const QColor& = Qt::green);
            void addText(const QString&, int = 0, int = 0);
            void addButtons(int s, const QStringList&); // Adds font icons
            void addButtons(int w, int h, bool, const QStringList&); // Adds pixmap icons
            QWidget* getButtons();

        private:
            Utils::MusicAdaptor *adaptor = nullptr; // Music controls and info

            bool showProgress = false;
            bool showText = false;
            QHBoxLayout layoutContainer;
            std::unique_ptr<QWidget> buttons;
            std::unique_ptr<QHBoxLayout> buttonsContainer;
            std::unique_ptr<Widgets::Progress> progress;
            std::unique_ptr<Widgets::Text> displayText;
            QString oldInfo;
            void addButtons(Widgets::Icon::IconType, bool, int, int, const QStringList&);
            void updateWidget();
    };
}

#endif // MUSICDISPLAY_H
