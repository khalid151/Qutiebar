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

        xcb_connection_t *conn = nullptr;
        xcb_ewmh_connection_t ewmh;
        xcb_window_t root;
        const unsigned int mask{XCB_EVENT_MASK_PROPERTY_CHANGE};

        public:
            X11EventHandler();
            ~X11EventHandler();

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

        public slots:
            void stop();
    };
}

#endif // X11HANDLER_H
