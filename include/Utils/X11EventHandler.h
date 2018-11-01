#ifndef X11HANDLER_H
#define X11HANDLER_H

#include <QIcon>
#include <QObject>
#include <QStringList>
#include <QThread>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xproto.h>

namespace Utils
{
    class X11EventHandler : public QThread
    {
        Q_OBJECT

        bool titleEvents, desktopEvents, classEvents, idEvents;
        xcb_connection_t *conn;
        xcb_ewmh_connection_t *ewmh;
        xcb_window_t root;
        const unsigned int mask{XCB_EVENT_MASK_PROPERTY_CHANGE};
        xcb_generic_event_t *event = nullptr;

        public:
            X11EventHandler(QObject *obj = nullptr);
            ~X11EventHandler();

            inline void trackWinId() { idEvents = true; }
            inline void trackTitle() { titleEvents = true; }
            inline void trackClass() { classEvents = true; }
            inline void trackWorkspace() { desktopEvents = true; }

            // Public just in case they become handy
            QIcon getIcon(xcb_window_t);
            QString getTitle(xcb_window_t);
            QString getWindowClass(xcb_window_t);

        protected:
            void run();

        private:
            bool running = true;
            void track(xcb_window_t, bool);
            void getWindowID(xcb_window_t*);
            void getCurrentDesktop();

        signals:
            void winIdChanged(unsigned int);
            void titleChanged(const QString &result);
            void classChanged(const QString &result);
            void desktopChanged(const QStringList &desktops, int index);
            void connectionLost();

        public slots:
            void stop();
    };
}

#endif // X11HANDLER_H
