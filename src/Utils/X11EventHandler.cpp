#include <QApplication>
#include <QImage>
#include <QPixmap>

#include "Utils/X11EventHandler.h"

namespace Utils
{
    X11EventHandler::X11EventHandler(QObject *parent) : QThread(parent)
    {
        conn = xcb_connect(nullptr, nullptr);
        ewmh = new xcb_ewmh_connection_t;
        xcb_ewmh_init_atoms_replies(ewmh, xcb_ewmh_init_atoms(conn, ewmh), nullptr);
        root = xcb_setup_roots_iterator(xcb_get_setup(conn)).data->root;
        track(root, true);
        xcb_flush(conn);
        connect(qApp, &QCoreApplication::aboutToQuit, this, &X11EventHandler::stop);
    }

    X11EventHandler::~X11EventHandler()
    {
        xcb_disconnect(conn);
        xcb_ewmh_connection_wipe(ewmh);
        free(ewmh);
        stop();
    }

    QIcon
    X11EventHandler::getIcon(xcb_window_t win)
    {
        QIcon icon;
        if(win == XCB_NONE) return icon; // If there's no window in focus

        xcb_ewmh_get_wm_icon_reply_t reply;
        xcb_ewmh_get_wm_icon_reply(ewmh, xcb_ewmh_get_wm_icon(ewmh, win), &reply, nullptr);
        if(reply.num_icons == 0) return icon; // In case there are no icons.
        auto iter = xcb_ewmh_get_wm_icon_iterator(&reply);
        while(iter.index < reply.num_icons)
        {
            QImage tmp(reinterpret_cast<const unsigned char*>(iter.data),
                    iter.width, iter.height, QImage::Format_ARGB32);
            if(tmp.isNull()) return icon;
            icon.addPixmap(QPixmap::fromImage(tmp));
            if(iter.rem > 0) xcb_ewmh_get_wm_icon_next(&iter);
            else break;
        }
        xcb_ewmh_get_wm_icon_reply_wipe(&reply);

        return icon;
    }

    QString
    X11EventHandler::getTitle(xcb_window_t win)
    {
        xcb_ewmh_get_utf8_strings_reply_t reply{};
        xcb_ewmh_get_wm_name_reply(ewmh, xcb_ewmh_get_wm_name(ewmh, win), &reply, nullptr);
        QString title = QString::fromUtf8(reply.strings, reply.strings_len);
        xcb_ewmh_get_utf8_strings_reply_wipe(&reply);
        return title;
    }

    QString
    X11EventHandler::getWindowClass(xcb_window_t win)
    {
        xcb_get_property_cookie_t class_cookie = xcb_icccm_get_wm_class(conn, win);
        xcb_get_property_reply_t *reply = xcb_get_property_reply(conn, class_cookie, nullptr);
        if(reply == nullptr)
            return "";
        auto windowClass = QString::fromUtf8(static_cast<char*>(xcb_get_property_value(reply)),
                xcb_get_property_value_length(reply));
        free(reply);
        if(windowClass.isEmpty())
            return "";
        try{
            return windowClass.split('\0')[1];
        } catch(...) {
            return windowClass.split('\0')[0];
        }
    }

    // Private
    void
    X11EventHandler::run()
    {
        xcb_window_t win = XCB_NONE;
        getWindowID(&win);
        track(win, true);
        emit titleChanged(getTitle(win));
        emit classChanged(getWindowClass(win));
        emit winIdChanged(win);
        getCurrentDesktop();
        while(running)
        {
            event = xcb_wait_for_event(conn);
                if(event->response_type == XCB_PROPERTY_NOTIFY && titleEvents)
                {
                    auto *notifyEvent = (xcb_property_notify_event_t*)event;
                    if (notifyEvent->atom == ewmh->_NET_ACTIVE_WINDOW)
                    {
                        track(win, false);
                        getWindowID(&win);
                        emit titleChanged(getTitle(win));
                        if(classEvents)
                            emit classChanged(getWindowClass(win));
                        if(idEvents)
                            emit winIdChanged(win);
                        track(win, true);
                    } else if (notifyEvent->atom == ewmh->_NET_WM_NAME || notifyEvent->atom == ewmh->_NET_WM_VISIBLE_NAME) {
                        getWindowID(&win);
                        emit titleChanged(getTitle(win));
                    } else if (notifyEvent->atom == ewmh->_NET_CURRENT_DESKTOP && desktopEvents) {
                        getCurrentDesktop();
                    }
                }
                free(event);
        }
    }

    void
    X11EventHandler::track(xcb_window_t win, bool enable)
    {
        if(win != XCB_NONE)
        {
            const unsigned int value{enable? XCB_EVENT_MASK_PROPERTY_CHANGE : XCB_EVENT_MASK_NO_EVENT};
            xcb_change_window_attributes(conn, win, XCB_CW_EVENT_MASK, &value);
        }
        getTitle(win); // I'm not completely sure why calling this function makes title tracking work :/
    }

    void
    X11EventHandler::getWindowID(xcb_window_t *win)
    {
        xcb_ewmh_get_active_window_reply(ewmh, xcb_ewmh_get_active_window(ewmh, 0), win, nullptr);
    }

    void
    X11EventHandler::getCurrentDesktop()
    {
        // TODO: emit signal when a window is moved to an empty workspace
        // also, urgent flags?
        unsigned int index = XCB_NONE;
        xcb_ewmh_get_utf8_strings_reply_t reply{};
        xcb_ewmh_get_desktop_names_reply(ewmh, xcb_ewmh_get_desktop_names(ewmh, 0), &reply, nullptr);
        xcb_ewmh_get_current_desktop_reply(ewmh, xcb_ewmh_get_current_desktop(ewmh, 0), &index, nullptr);
        QString desktops = QString::fromUtf8(reply.strings, reply.strings_len);
        xcb_ewmh_get_utf8_strings_reply_wipe(&reply);
        auto stringList = desktops.split('\0');
        stringList.removeLast();
        emit desktopChanged(stringList, index);
    }

    void
    X11EventHandler::stop()
    {
        running = false;
        // xcb_wait_for_event() is blocking until it gets an event, so here's an event :P
        xcb_send_event(conn, false, root, XCB_EVENT_MASK_NO_EVENT, "");
        xcb_flush(conn);
        quit();
        wait();
    }
}
