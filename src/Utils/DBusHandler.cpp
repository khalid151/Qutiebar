#include "Utils/DBusHandler.h"
#include "Utils/Misc.h" // For getColor() : string -> QColor

namespace Utils
{
    DBusHandler::DBusHandler(Builder *b, QObject *p) : QObject(p)
    {
        builder = b;
    }

    void
    DBusHandler::setPanelBackground(const QString &p, const QString &c)
    {
        auto panel = builder->getPanel(p);
        if(panel != nullptr)
            panel->setBackground(getColor(c));
    }

    void
    DBusHandler::setPanelForeground(const QString &p, const QString &c)
    {
        auto panel = builder->getPanel(p);
        if(panel != nullptr)
            panel->setForeground(getColor(c));
    }

    void
    DBusHandler::setPanelUnderline(const QString &p, int w, const QString &c)
    {
        auto panel = builder->getPanel(p);
        if(panel != nullptr)
            panel->setUnderline(w, getColor(c));
    }

    void
    DBusHandler::setPanelOverline(const QString &p, int w, const QString &c)
    {
        auto panel = builder->getPanel(p);
        if(panel != nullptr)
            panel->setOverline(w, getColor(c));
    }

    void
    DBusHandler::setPanelMargins(const QString &p, int m)
    {
        auto panel = builder->getPanel(p);
        if(panel != nullptr)
            panel->setMargins(m);
    }

    void
    DBusHandler::setPanelRadius(const QString &p, int r)
    {
        auto panel = builder->getPanel(p);
        if(panel != nullptr)
            panel->setRoundCorners(r);
    }

    void
    DBusHandler::setModuleBackground(const QString &m, const QString &c)
    {
        auto module = builder->getModule(m);
        if(module != nullptr)
            module->setBackground(getColor(c));
    }

    void
    DBusHandler::setModuleForeground(const QString &m, const QString &c)
    {
        auto module = builder->getModule(m);
        if(module != nullptr)
            module->setForeground(getColor(c));
    }

    void
    DBusHandler::setModuleUnderline(const QString &m, int w, const QString &c)
    {
        auto module = builder->getModule(m);
        if(module != nullptr)
            module->setUnderline(w, getColor(c));
    }

    void
    DBusHandler::setModuleOverline(const QString &m, int w, const QString &c)
    {
        auto module = builder->getModule(m);
        if(module != nullptr)
            module->setOverline(w, getColor(c));
    }

    void
    DBusHandler::setModuleRadius(const QString &m, int r)
    {
        auto module = builder->getPanel(m);
        if(module != nullptr)
            module->setRoundCorners(r);
    }
}
