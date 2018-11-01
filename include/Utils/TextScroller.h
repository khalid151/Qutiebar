#ifndef TEXTSCROLLER_H
#define TEXTSCROLLER_H

#include <QObject>
#include <QThread>
#include <QString>

namespace Utils
{
    class Scroller : public QThread
    {
        Q_OBJECT
        public:
            Scroller(bool = false, QObject* = nullptr);

            void setText(const QString&, const int);
            void setScrollSpeed(const float);
            void stop();

        protected:
            void run();

        private:
            bool shouldStop = false;
            bool richText = false;
            int maxLength;
            int pause = 250;
            QString text;

        signals:
            void sendText(const QString&);
    };
}

#endif // TEXTSCROLLER_H
