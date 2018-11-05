#ifndef BUILDER_H
#define BUILDER_H

#include <memory>
#include <vector>
#include <map>
#include <QProcess>
#include <QStringList>
#include <QSettings>
#include <QWidget>

#include "Modules/AppIcon.h"
#include "Modules/Clock.h"
#include "Modules/Desktops.h"
#include "Modules/DisplayItem.h"
#include "Widgets/Panel.h"
#include "Utils/Misc.h"
#include "Utils/X11EventHandler.h"

#ifdef ENABLE_MPD
#include "Utils/MpdConnection.h"
#endif

#ifdef ENABLE_DBUS
#include "Utils/Mpris.h"
#endif

#if defined(ENABLE_MPD) || defined(ENABLE_DBUS)
#include "Modules/MusicDisplay.h"
#endif

namespace Utils
{
    class Builder
    {
        public:
            Builder(const QString&);

            static QString defaultConfigPath();
            Widgets::Panel* getPanel(const QString&); // Used in Utils/DBusHandler
            Utils::WidgetProperties* getModule(const QString&); // Used in Utils/DBusHandler

            void build();

        private:
            bool useXEvents = false; // If true, X11EventHandler thread will be started
            std::map<const QString, QWidget*> widgets; // Keep track of widgets to change their properties later
            std::map<const QString, QColor> colors; // For [colors] in config if used.
            std::unique_ptr<QSettings> config;
            std::unique_ptr<Utils::X11EventHandler> xevents;
            std::vector<std::unique_ptr<Utils::DataModel>> dataList; // To store "data" objects.
            std::vector<std::unique_ptr<Widgets::Panel>> panelList;
            std::vector<std::unique_ptr<QProcess>> procList;
            std::vector<std::unique_ptr<QWidget>> moduleList;

#ifdef ENABLE_MPD
            std::unique_ptr<Utils::MpdConnection> mpdConn;
#endif
#ifdef ENABLE_DBUS
            std::unique_ptr<Utils::Mpris> mprisInterface;
#endif

            QString configDir();

            QColor getConfiguredColor(const QString&); // Takes name of configuration key as a parameter
            void configureProgress(Widgets::Progress*);
            void setProperties(Utils::WidgetProperties*);

            QStringList generateItemsList(const QString&); // To get list of panels\groups\modules
            QStringList generateModulesList();
            bool hasModule(const QString&);

            std::unique_ptr<Modules::DisplayItem> buildDisplayItem(Utils::DataModel*);
            void buildModules();
            void buildCustomModules(const QString&); // Called inside buildModules();
            void buildGroups();
            void buildPanels();
    };
}

#endif // BUILDER_H
