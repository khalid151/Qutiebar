#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QEvent>
#include <QObject>

namespace Utils
{
    class EventHandler : public QObject
    {
        Q_OBJECT

        public:
            EventHandler(QObject *parent = nullptr);
            bool eventFilter(QObject*, QEvent*);

        signals:
            void MouseEnter();
            void MouseLeave();
            void MouseClick();
    };
}

#endif // EVENTHANDLER_H
