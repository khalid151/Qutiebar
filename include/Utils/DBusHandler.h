#ifndef DBUSHANDLER_H
#define DBUSHANDLER_H

#include <QObject>

#include "Widgets/Panel.h"
#include "Utils/Builder.h"
#include "Utils/WidgetProperties.h"

namespace Utils
{
    class DBusHandler : public QObject
    {
        Q_OBJECT

        public:
            DBusHandler(Builder*, QObject* = nullptr);

        private:
            Builder *builder; // To query panels\modules

        public slots:
            void setPanelBackground(const QString &panel, const QString &color);
            void setPanelForeground(const QString &panel, const QString &color);
            void setPanelUnderline(const QString &panel, int width, const QString &color);
            void setPanelOverline(const QString &panel, int width, const QString &color);
            void setPanelMargins(const QString &panel, int margin);
            void setPanelRadius(const QString &panel, int radius);

            void setModuleBackground(const QString &module, const QString &color);
            void setModuleForeground(const QString &module, const QString &color);
            void setModuleUnderline(const QString &module, int width, const QString &color);
            void setModuleOverline(const QString &module, int width, const QString &color);
            void setModuleRadius(const QString &module, int radius);
    };
}

#endif // DBUSHANDLER_H
