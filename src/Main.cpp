#include "Version.h"

#include <cstdio>
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QResource>
#include <QSettings>

#ifdef ENABLE_DBUS
#include <QDBusConnection>
#include "Utils/DBusHandler.h"
#endif

#include "Utils/Builder.h"
#include "Utils/UnixSignalHandler.h"

int main(int argc, char **argv)
{
    QString configPath;
    QResource::registerResource("res/resources.qrc");
    QApplication app(argc, argv);
    QApplication::setApplicationName("Qutiebar");
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QCommandLineParser parser;
    parser.setApplicationDescription("A customizable panel based on Qt.");
    parser.addHelpOption();
    parser.addOptions({
            {QStringList{"c", "config"}, "Load a configuration file.", "config-file"},
            {QStringList{"v", "version"}, "Display version number and exit."},
            });
    parser.process(app);

    if(parser.isSet("config")) configPath = parser.value("config");
    else configPath = Utils::Builder::defaultConfigPath();

    if(parser.isSet("v"))
    {
        printf("Qutiebar version: %s\n", VERSION);
        return 0;
    }

#ifdef ENABLE_DBUS
    // D-Bus -- register serivce before building widgets, there might be scripts needing that
    QDBusConnection::sessionBus().registerService("Qutiebar.panel");
#endif

    // Build modules
    Utils::Builder builder(configPath);
    builder.build();

#ifdef ENABLE_DBUS
    // Expose this object to manipulate properties
    Utils::DBusHandler bus(&builder);
    QDBusConnection::sessionBus().registerObject("/", "panel.settings", &bus, QDBusConnection::ExportAllSlots);
#endif

    Utils::UnixSignalHandler sig;
    QObject::connect(&sig, &Utils::UnixSignalHandler::stop, &app, &QCoreApplication::quit);
    QObject::connect(qApp, &QApplication::aboutToQuit, qApp, [](){ fprintf(stdout, "Exit.\n"); });

    return app.exec();
}
