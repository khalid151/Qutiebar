#include <cstdio>
#include <functional>
#include <QApplication>
#include <QDir>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QScreen>
#include <QTimer>

#include "Data/Battery.h"
#include "Data/Backlight.h"
#include "Data/Process.h"
#include "Modules/Group.h"
#include "Utils/Builder.h"

#ifdef ENABLE_ALSA
#include "Data/Volume.h"
#endif

#include <QtX11Extras/QX11Info>

namespace Utils
{
    Builder::Builder(const QString &location)
    {
        config = std::make_unique<QSettings>(location, QSettings::IniFormat);
        config->setIniCodec("UTF-8");
        xevents = std::make_unique<Utils::X11EventHandler>();
    }

    QString
    Builder::defaultConfigPath()
    {
        auto xdgConf = QString(qgetenv("XDG_CONFIG_HOME"));
        auto home = QString(qgetenv("HOME"));
        auto configDir = xdgConf.isEmpty()? QString("%1/.config").arg(home) : xdgConf;

        QFile config;
        config.setFileName(QString("%1/Qutiebar/config").arg(configDir));
        if(config.exists())
            return config.fileName();

        fprintf(stderr, "Couldn't find default configuration file.\n");
        exit(1);
    }

    Widgets::Panel*
    Builder::getPanel(const QString &panel)
    {
        return dynamic_cast<Widgets::Panel*>(widgets[panel]);
    }

    Utils::WidgetProperties*
    Builder::getModule(const QString &module)
    {
        return dynamic_cast<Utils::WidgetProperties*>(widgets[module]);
    }

    void
    Builder::build()
    {
        // Some general configuration before building stuff
        config->beginGroup("global");
        QIcon::setThemeName(config->value("icon-theme", "hicolor").toString());
        config->endGroup();

        // Generating a color map thing
        config->beginGroup("colors");
        for(const auto &name:config->childKeys())
        {
            auto color = Utils::getColor(config->value(name).toStringList().join(","));
            colors.insert(std::make_pair(name, color));
        }
        config->endGroup();

        buildModules();
        buildGroups();
        buildPanels();
        if(useXEvents) xevents->start();
    }

    // Private
    QString
    Builder::configDir()
    {
        auto configPath = config->fileName();
        return configPath.replace(QRegularExpression("[^/]+(?=/$|$)"), "");
    }

    // Configuration functions
    QColor
    Builder::getConfiguredColor(const QString &c)
    {
        QColor color;
        auto name = config->value(c).toStringList().join(",");
        if(name.startsWith("$")) color = colors[name.mid(1)];
        else color = Utils::getColor(name);

        if(color.isValid()) return color;
        else return Qt::transparent;
    }

    void
    Builder::configureProgress(Widgets::Progress *p)
    {
        if(p == nullptr) return;

        auto filled = config->contains("progress-color") ?
            getConfiguredColor("progress-color") : Qt::white;
        auto empty = config->contains("progress-empty-color") ?
            getConfiguredColor("progress-empty-color") : Qt::transparent;
        int width = config->value("progress-width", 2).toInt();
        if(config->contains("progress-empty-width"))
        {
            int emptyWidth = config->value("progress-empty-width").toInt();
            p->setStyle(width, filled, empty, filled, emptyWidth);
        }
        else
            p->setStyle(width, filled, empty, filled);

        auto valign = config->value("progress-vertical-align", 0).toInt();
        p->setVerticalAlignment(valign);

        if(config->contains("progress-font-size"))
            p->setFontSize(config->value("progress-font-size").toInt());
        if(config->contains("progress-font-color"))
            p->setFontColor(getConfiguredColor("progress-font-color"));
        if(config->contains("circle-icon"))
            p->setCircleIcon(config->value("circle-icon").toString());
    }

    void
    Builder::setProperties(Utils::WidgetProperties *props)
    {
        if(config->contains("background"))
            props->setBackground(getConfiguredColor("background"));
        if(config->contains("foreground"))
            props->setForeground(getConfiguredColor("foreground"));

        if(config->contains("font")) props->setFontFamily(config->value("font").toString());
        else if(config->contains("fonts")) props->setFontFamily(config->value("fonts").toStringList());
        if(config->contains("font-size")) props->setFontSize(config->value("font-size").toInt());

        if(config->contains("shadow-color") || config->contains("shadow-radius"))
        {
            auto c = config->contains("shadow-color") ?
                getConfiguredColor("shadow-color") : Qt::black;
            int r = config->value("shadow-radius", 5).toInt();
            int x = config->value("shadow-x", 0).toInt();
            int y = config->value("shadow-y", 0).toInt();
            props->setShadow(r, x, y, c);
        }

        if(config->contains("radius")) props->setRoundCorners(config->value("radius").toInt());

        if(config->contains("border"))
        {
            auto c = config->contains("border-color") ?
                getConfiguredColor("border-color") : Qt::red;
            int w = config->value("border").toInt();
            props->setBorder(w, c);
        }

        for(const auto &line:QStringList{"underline", "overline"})
        {
            if(config->contains(line))
            {
                int width = config->value(line).toInt();
                auto lineprop = QString(line).append("-color");
                auto color = config->contains(lineprop) ?
                    getConfiguredColor(lineprop) : Qt::red;
                if(line == "underline") props->setUnderline(width, color);
                else props->setOverline(width, color);
            }
        }

        if(config->contains("vertical-align"))
            props->setVerticalAlignment(config->value("vertical-align").toInt());
    }

    // List generating functions
    QStringList
    Builder::generateItemsList(const QString &type)
    {
        // Used to get list of "groups" or "panels"
        QStringList list;
        for(const auto &c:config->allKeys())
            if(c.contains(type))
            {
                auto item = c.left(c.lastIndexOf(QRegularExpression("[^/]+")) - 1);
                if(!list.contains(item))
                    list.append(item);
            }
        return list;
    }

    QStringList
    Builder::generateModulesList()
    {
        QStringList modules;

        // Adding modules\groups for each created panel
        for(const auto &p:generateItemsList("panel"))
        {
            config->beginGroup(p);
            for(const auto &type:QStringList{"modules-left", "modules-center", "modules-right", "scripts"})
                for(const auto &m:config->value(type).toStringList())
                    if(!modules.contains(m))
                        modules.append(m);
            config->endGroup();
        }

        // Adding modules for each group added in previous panels
        for(const auto &g:generateItemsList("group"))
        {
            if(!modules.contains(QString(g).replace("group/", "")))
                continue; // Skip modules for groups that are created but not added
            config->beginGroup(g);
            for(const auto &m:config->value("modules").toStringList())
                if(!modules.contains(m))
                    modules.append(m);
            config->endGroup();
        }

        return modules;
    }

    bool
    Builder::hasModule(const QString &n)
    {
        for(const auto &k:config->allKeys())
            if(k.contains(n)) return true;
        return false;
    }

    // Building functions
    std::unique_ptr<Modules::DisplayItem>
    Builder::buildDisplayItem(Utils::DataModel *M)
    {
        using namespace Modules;
        int padding = config->value("padding", 0).toInt();
        int icon_padding = config->value("icon-padding", 0).toInt();
        int data_padding = config->value("data-padding", 0).toInt();
        auto displayItem = std::make_unique<DisplayItem>(M, padding);
        setProperties(displayItem.get());

        int width = config->value("icon-width", 20).toInt();
        int height = config->value("icon-height", 20).toInt();
        int size = config->value("icon-size", 10).toInt();
        int radius = config->value("progress-radius", 20).toInt();
        int length = config->value("progress-length", 100).toInt();

        auto icon = config->value("icon").toString().toLower();
        auto data = config->value("data", "number").toString().toLower();
        auto location = config->value("icon-location", "left").toString().toLower();

        auto icons = config->value("icons").toStringList();
        displayItem->setIcons(std::move(icons));

        auto addData = [radius, length, data, data_padding](auto d) {
            if(data == "number")
                d->addDataDisplay(0, 0, data_padding);
            else if(data == "circle")
                d->addDataDisplay(radius, 0, data_padding);
            else if(data == "line")
                d->addDataDisplay(0, length, data_padding);
        };

        if(location == "left")
        {
            displayItem->addIconDisplay(icon, width, height, size, icon_padding);
            addData(displayItem.get());
        }
        else if(location == "right")
        {
            addData(displayItem.get());
            displayItem->addIconDisplay(icon, width, height, size, icon_padding);
        }

        if(data == "circle" || data == "line")
            configureProgress(dynamic_cast<Widgets::Progress*>(displayItem->data()));

        auto iconBG = config->contains("icon-background") ?
            getConfiguredColor("icon-background") : Qt::transparent;
        auto dataBG = config->contains("data-background") ?
            getConfiguredColor("data-background") : Qt::transparent;

        auto dataProps = std::make_unique<Utils::WidgetProperties>(displayItem->data(), false);
        dataProps->setBackground(dataBG);
        if(config->contains("data-color"))
            dataProps->setForeground(getConfiguredColor("data-color"));

        if(displayItem->icon() != nullptr)
        {
            auto iconProps = std::make_unique<Utils::WidgetProperties>(displayItem->icon(), false);
            iconProps->setBackground(iconBG);
            if(config->contains("icon-color"))
                iconProps->setForeground(getConfiguredColor("icon-color"));
        }

        return displayItem;
    }

    void
    Builder::buildModules()
    {
        auto modules = generateModulesList(); // Just what panels need

        for(const auto &name:modules)
        {
            if(!hasModule(name))
            {
                fprintf(stderr, "Could not find %s\n", name.toLatin1().data());
                continue; // Skip if it doesn't have the module
            }

            auto type = name.left(name.indexOf("-"));
            config->beginGroup(QString("module/%1").arg(name));

            int width = config->value("width", 25).toInt();
            int height = config->value("height", 25).toInt();
            int padding = config->value("padding", 0).toInt();
            float update = config->value("update-rate", 1).toFloat();

            if(type == "appicon")
            {
                useXEvents = true;
                auto icon = std::make_unique<Modules::AppIcon>(xevents.get(), width, height, padding);
                auto aa = config->value("antialiasing", true).toBool();
                icon->setAntialiasing(aa, aa);
                setProperties(icon.get());
                if(config->contains("default-icon"))
                    icon->setDefaultIcon(config->value("default-icon").toString());

                widgets.insert(std::make_pair(name, icon.get()));
                moduleList.push_back(std::move(icon));
            }
            else if(type == "title")
            {
                useXEvents = true;
                auto title = std::make_unique<Widgets::Text>("", padding);
                setProperties(title.get());

                auto wm_class = config->value("class", false).toBool();
                QObject::connect(xevents.get(), wm_class ?
                        &Utils::X11EventHandler::classChanged : &Utils::X11EventHandler::titleChanged,
                        title.get(), &Widgets::Text::updateText);

                if(config->value("rich-text", false).toBool())
                    title->enableRichText();

                if(config->contains("max-length"))
                {
                    int maxlen = config->value("max-length").toInt();
                    float speed = config->value("scroll-speed", 0.25).toFloat();
                    title->setMaxLength(maxlen, speed);
                }

                widgets.insert(std::make_pair(name, title.get()));
                moduleList.push_back(std::move(title));
            }
            else if(type == "clock")
            {
                auto format = config->value("format", "hh:mm a").toString();
                auto clock = std::make_unique<Modules::Clock>(padding, format, static_cast<int>(update * 1000));
                clock->setAltFormat(config->value("alt-format", format).toString());

                if(config->contains("time-zone"))
                    clock->setTimeZone(config->value("time-zone").toString());

                setProperties(clock.get());

                widgets.insert(std::make_pair(name, clock.get()));
                moduleList.push_back(std::move(clock));
            }
            else if(type == "workspace")
            {
                useXEvents = true;
                Widgets::IconType::IconType ic;
                auto iconType = config->value("icon-type", "fonticon").toString().toLower();
                if(iconType == "fonticon" || iconType == "text") ic = Widgets::IconType::Text;
                else if(iconType == "pixmap") ic = Widgets::IconType::Pixmap;
                // Current -> only one workspace displayed at a time
                auto t = config->value("current-only", false).toBool()? Modules::Desktops::Current
                    : Modules::Desktops::Populated;

                auto iconList = config->value("icons").toStringList();
                if(config->contains("default-icon"))
                    iconList.append(QString("default:%1").arg(config->value("default-icon").toString()));

                auto desktops = std::make_unique<Modules::Desktops>(t, ic, std::move(iconList), padding);

                auto aa = config->value("antialiasing", true).toBool();
                setProperties(desktops.get());
                if(ic == Widgets::IconType::Pixmap)
                {
                    desktops->resize(width, height);
                    desktops->setAntialiasing(aa, aa);
                }

                desktops->setSpacing(config->value("spacing", 5).toInt());

                // Setting indicators
                Modules::Desktops::Lines l;
                QColor foreground = Qt::white, background = Qt::transparent, line = Qt::red;
                if(config->contains("active-color"))
                    foreground = getConfiguredColor("active-color");
                if(config->contains("active-background"))
                    background = getConfiguredColor("active-background");

                if(config->contains("active-underline-color")) {
                    line = getConfiguredColor("active-underline-color");
                    l= Modules::Desktops::Underline;
                } else if(config->contains("active-overline-color")) {
                    line = getConfiguredColor("active-overline-color");
                    l= Modules::Desktops::Overline;
                } else
                    l= Modules::Desktops::None;

                desktops->setIndicatorStyle(Modules::Desktops::Active,
                        std::move(foreground), std::move(background),
                        std::move(line), config->value("active-line-width", 0).toInt(), l);

                foreground = config->contains("inactive-color") ?
                    getConfiguredColor("inactive-color") : foreground;
                background = config->contains("inactive-background") ?
                    getConfiguredColor("inactive-background") : Qt::transparent;

                if(config->contains("inactive-underline-color")) {
                    line = getConfiguredColor("inactive-underline-color");
                    l= Modules::Desktops::Underline;
                } else if(config->contains("inactive-overline-color")) {
                    line = getConfiguredColor("inactive-overline-color");
                    l= Modules::Desktops::Overline;
                } else
                    l= Modules::Desktops::None;

                if(config->contains("font-size"))
                    desktops->resize(config->value("font-size").toInt());

                desktops->setIndicatorStyle(Modules::Desktops::Inactive,
                        std::move(foreground), std::move(background),
                        std::move(line), config->value("inactive-line-width", 0).toInt(), l);

                QObject::connect(xevents.get(), &Utils::X11EventHandler::desktopChanged,
                        desktops.get(), &Modules::Desktops::updateDesktops);

                widgets.insert(std::make_pair(name, desktops.get()));
                moduleList.push_back(std::move(desktops));
            }
            else if(type == "battery")
            {
                auto battery = QString("/sys/class/power_supply/%1").arg(config->value("battery", "BAT0").toString());
                auto batteryData = std::make_unique<Data::Battery>(battery, static_cast<int>(update * 1000));
                auto batteryItem = buildDisplayItem(batteryData.get());

                auto charging = config->contains("charging-color") ?
                    getConfiguredColor("charging-color") : Qt::green;
                auto discharging = config->contains("discharging-color") ?
                    getConfiguredColor("discharging-color") : Qt::black;

                auto icons = config->value("charging-icons").toStringList();
                auto sIcons = config->value("discharging-icons").toStringList();
                batteryItem->setIcons(std::move(icons), std::move(sIcons));

                batteryItem->setColors(std::move(charging), std::move(discharging));

                widgets.insert(std::make_pair(name, batteryItem.get()));
                moduleList.push_back(std::move(batteryItem));
                dataList.push_back(std::move(batteryData));
            }
#ifdef ENABLE_ALSA
            else if(type == "volume")
            {
                auto soundcard = config->value("soundcard", "default").toString();
                auto mixer = config->value("mixer", "Master").toString();
                auto volumeData = std::make_unique<Data::Volume>(
                        soundcard, mixer, static_cast<int>(update * 1000));
                auto volumeItem = buildDisplayItem(volumeData.get());

                widgets.insert(std::make_pair(name, volumeItem.get()));
                moduleList.push_back(std::move(volumeItem));
                dataList.push_back(std::move(volumeData));
            }
#endif
            else if(type == "backlight")
            {
                auto backlight = config->value("backlight", "intel_backlight").toString();
                auto backlightData = std::make_unique<Data::Backlight>(
                        backlight.prepend("/sys/class/backlight/"), static_cast<int>(update * 1000));
                auto backlightItem = buildDisplayItem(backlightData.get());

                widgets.insert(std::make_pair(name, backlightItem.get()));
                moduleList.push_back(std::move(backlightItem));
                dataList.push_back(std::move(backlightData));
            }
#if defined(ENABLE_MPD) || defined(ENABLE_DBUS)
            else if(type == "mpd" || type == "mpris")
            {
                std::unique_ptr<Modules::MusicDisplay> music;
#ifdef ENABLE_MPD
                if(type == "mpd")
                {
                    auto host = config->value("host", "127.0.0.1").toString();
                    auto port = config->value("port", 6600).toInt();
                    auto password = config->value("password", "").toString();
                    auto timeout = config->value("timeout", 1000).toInt();

                    if(!Utils::MpdConnection::hasInstance)
                        mpdConn = std::make_unique<Utils::MpdConnection>(host, port, password, timeout);
                    if(!mpdConn->couldConnect())
                    {
                        config->endGroup();
                        mpdConn.reset();
                        continue; // Don't add module if couldn't connect
                    }

                    music = std::make_unique<Modules::MusicDisplay>(mpdConn.get(), update * 1000);
                }
#endif
#ifdef ENABLE_DBUS
                if(type == "mpris")
                {
                    auto musicPlayer = config->value("music-player").toString();
                    if(musicPlayer.isEmpty())
                    {
                        config->endGroup();
                        continue; // Don't add module if there's no player specified
                    }

                    auto service = QString("org.mpris.MediaPlayer2.%1").arg(musicPlayer);
                    if(!Utils::Mpris::hasInstance)
                        mprisInterface = std::make_unique<Utils::Mpris>(service);

                    music = std::make_unique<Modules::MusicDisplay>(mprisInterface.get(), update * 1000);
                }
#endif

                // If mpd is disabled but still included in config file
                if(music == nullptr)
                {
                    config->endGroup();
                    continue;
                }

                setProperties(music.get());

                auto buttons = config->value("add-buttons").toStringList();
                auto textFmt = config->value("text-format").toString();
                auto maxlen = config->value("text-max-length", 0).toInt();
                auto len = config->value("progress-length", 100).toInt();
                auto valign = config->value("progress-vertical-align", 0).toInt();
                int width = config->value("button-width", 20).toInt();
                int height = config->value("button-height", 20).toInt();
                int iconSize = config->value("button-size", 10).toInt();
                bool richText = config->value("rich-text", false).toBool();
                float speed = config->value("scroll-speed", 0.25).toFloat();

                QStringList defaultOrder{"text", "buttons", "progress"};
                auto order = config->value("order", defaultOrder).toStringList();

                for(const auto &o:order) {
                    if(o == "text")
                    {
                        if(config->contains("text-format"))
                        music->addText(textFmt, maxlen, speed, padding, richText);
                    }
                    else if(o == "buttons")
                    {
                        if(config->contains("add-buttons"))
                        {
                            bool aa = config->value("antialiasing", true).toBool();
                            auto type = config->value("button-type", "fonticon").toString().toLower();
                            if(type == "pixmap")
                                music->addButtons(width, height, aa, buttons);
                            else if(type == "fonticon" || type == "text")
                                music->addButtons(iconSize, buttons);
                            auto props = std::make_unique<Utils::WidgetProperties>(music->buttons());
                            if(config->contains("button-color"))
                                props->setForeground(getConfiguredColor("button-color"));
                            if(config->contains("button-background"))
                                props->setBackground(getConfiguredColor("button-background"));
                        }
                    }
                    else if(o == "progress")
                    {
                        if(config->value("add-progress", false).toBool())
                        {
                            music->addProgress(len, 2, valign);
                            configureProgress(music->progress());
                        }
                    }
                }

                widgets.insert(std::make_pair(name, music.get()));
                moduleList.push_back(std::move(music));
            }
#endif // if def ENABLE_MPD || ENABLE_DBUS

            config->endGroup();

            buildCustomModules(name);
        }

    }

    void
    Builder::buildCustomModules(const QString &name)
    {
        QStringList types{"text", "icon", "data", "script"};
        for(const auto &type:types)
        {
            if(hasModule(QString("%1/%2").arg(type, name)))
            {
                config->beginGroup(QString("%1/%2").arg(type, name));

                using namespace Widgets;

                int padding = config->value("padding", 0).toInt();
                int width = config->value("width", 25).toInt();
                int height = config->value("height", 25).toInt();
                Utils::EventHandler *events;

                // Process properties
                bool one_shot = config->value("one-shot", true).toBool();
                auto update = config->value("update-rate", 1).toFloat();
                auto proc = std::make_unique<QProcess>();

                // Create a widget
                if(type == "text")
                {
                    auto readStdOut = [](auto p, auto w) {
                        w->setText(p->readAll().simplified());
                    };
                    auto label = config->value("label", "").toString();
                    auto text = std::make_unique<Text>(label, padding);
                    events = text->event;
                    setProperties(text.get());
                    if(config->contains("max-length"))
                    {
                        int maxlen = config->value("max-length").toInt();
                        float speed = config->value("scroll-speed", 0.25).toFloat();
                        text->setMaxLength(maxlen, speed);
                    }
                    if(config->value("rich-text", false).toBool())
                        text->enableRichText();

                    QObject::connect(proc.get(), &QProcess::readyReadStandardOutput,
                            text.get(), std::bind(readStdOut, proc.get(), text.get()));

                    widgets.insert(std::make_pair(name, text.get()));
                    moduleList.push_back(std::move(text));
                }
                else if(type == "icon")
                {
                    auto readStdOut = [](auto p, auto w) {
                        w->load(p->readAll().simplified());
                    };
                    auto defaultIcon = config->value("default-icon", ":empty.svg").toString();
                    auto icon = std::make_unique<Icon>(defaultIcon, width, height, padding);
                    events = icon->event;
                    setProperties(icon.get());
                    auto aa = config->value("antialiasing", true).toBool();
                    icon->setAntialiasing(aa, aa);
                    QObject::connect(proc.get(), &QProcess::readyReadStandardOutput,
                            icon.get(), std::bind(readStdOut, proc.get(), icon.get()));
                    widgets.insert(std::make_pair(name, icon.get()));
                    moduleList.push_back(std::move(icon));
                }
                else if(type == "data")
                {
                    auto data = std::make_unique<Data::Process>(proc.get());
                    int max = config->value("max-value", 100).toInt();
                    auto unit = config->value("unit", "%").toString();
                    data->setMax(max);
                    data->setUnit(unit);
                    if(config->contains("name"))
                        data->setName(config->value("name").toString());

                    auto display = buildDisplayItem(data.get());
                    if(config->value("icon").toString().contains("process"))
                        data->enableCustomIcon(display->icon());

                    events = display->event;
                    widgets.insert(std::make_pair(name, display.get()));
                    moduleList.push_back(std::move(display));
                    dataList.push_back(std::move(data));
                }

                if(config->contains("exec"))
                {
                    auto args = Utils::arguments(std::move(config->value("exec").toString()));
                    args.replaceInStrings(QRegularExpression("^~"), QString(qgetenv("HOME"))); // Expanding ~ to $HOME
                    if(!args.isEmpty())
                    {
                        auto command = args[0];
                        args.removeFirst();
                        if(one_shot) proc->start(command, args);
                        else
                        {
                            auto *timer = new QTimer(proc.get());
                            auto start = std::bind(
                                    [](auto p, auto c, auto a) {
                                        p->start(c, a);
                                    }, proc.get(), command, args);
                            QObject::connect(timer, &QTimer::timeout, proc.get(), start);
                            timer->start(static_cast<int>(update * 1000));
                        }
                    }
                }
                else if(config->contains("exec-on-click") && type != "script")
                {
                    auto args = Utils::arguments(std::move(config->value("exec-on-click").toString()));
                    args.replaceInStrings(QRegularExpression("^~"), QString(qgetenv("HOME"))); // Expanding ~ to $HOME
                    if(!args.isEmpty())
                    {
                        auto command = args[0];
                        args.removeFirst();
                        auto start = [](auto p, auto c, auto a) { p->start(c, a); };
                        QObject::connect(events, &Utils::EventHandler::MouseClick,
                                proc.get(), std::bind(start, proc.get(), command, args));
                    }
                }

                auto realKiller = std::bind([](auto p) { // Muahahaha
                            p->kill();
                            p->waitForFinished();
                        },  proc.get());

                QObject::connect(qApp, &QApplication::aboutToQuit, proc.get(), realKiller);
                procList.push_back(std::move(proc));

                config->endGroup();
            }
        }
    }

    void
    Builder::buildGroups()
    {
        for(const auto &g:generateItemsList("group"))
        {
            auto name = QString(g).replace("group/", "");
            config->beginGroup(g);
            int padding = config->value("padding", 0).toInt();
            auto group = std::make_unique<Modules::Group>(padding);
            setProperties(group.get());
            group->setMargins(config->value("inner-margins", 0).toInt());
            group->setSpacing(config->value("spacing", 0).toInt());

            for(const auto &m:config->value("modules").toStringList())
                if(widgets[m] != nullptr)
                {
                    widgets[m]->setParent(group.get());
                    group->addWidget(widgets[m]);
                }

            widgets.insert(std::make_pair(name, group.get()));
            moduleList.push_back(std::move(group));
            config->endGroup();
        }
    }

    void
    Builder::buildPanels()
    {
        for(const auto &p:generateItemsList("panel"))
        {
            auto name = QString(p).replace("panel/", "");
            config->beginGroup(p);

            QRect screen;
            if(config->contains("monitor"))
            {
                for(auto s:QGuiApplication::screens())
                    if(s->name() == config->value("monitor").toString())
                        screen = s->geometry();
                // Fallback if not found
                if(!screen.width() && !screen.height())
                    screen = QGuiApplication::screens()[0]->geometry();
            }
            else
                screen = QGuiApplication::screens()[0]->geometry();

            int width, height, padding, margins;
            auto w = config->value("width", "100%").toString();
            auto h = config->value("height", "4%").toString();
            if(w.contains('%')) width = static_cast<int>(w.replace('%', "").toFloat()/100 * screen.width());
            else width = w.toInt();
            if(h.contains('%')) height = static_cast<int>(h.replace('%', "").toFloat()/100 * screen.height());
            else height = h.toInt();
            padding = config->value("padding", 0).toInt();
            margins = config->value("margins", 0).toInt();

            auto up = std::make_unique<Widgets::Panel>(screen, width, height, margins, padding);
            panelList.push_back(std::move(up));
            auto panel = panelList.back().get();
            bool over = config->value("over-windows", true).toBool();
            bool bottom = config->value("bottom", false).toBool();
            if(bottom) panel->move(screen.x(), screen.y() + screen.height() - panel->height());
            else panel->move(screen.x(), screen.y());

            // Setting EWMH hints
            auto conn = QX11Info::connection();
            xcb_ewmh_connection_t ewmh;
            auto ewmh_cookie = xcb_ewmh_init_atoms(conn, &ewmh);
            xcb_ewmh_init_atoms_replies(&ewmh, ewmh_cookie, nullptr);

            xcb_atom_t states[2] = {ewmh._NET_WM_STATE_STICKY};
            if(over) states[1] = ewmh._NET_WM_STATE_ABOVE;
            else states[1] = ewmh._NET_WM_STATE_BELOW;
            xcb_atom_t type[] = {ewmh._NET_WM_WINDOW_TYPE_DOCK};
            xcb_ewmh_set_wm_state(&ewmh, panel->winId(), 2, states);
            xcb_ewmh_set_wm_window_type(&ewmh, panel->winId(), 1, type);

            // Get total height of desktop
            int desktopHeight = 0;
            for(auto s:QGuiApplication::screens())
            {
                int sum = s->geometry().y() + s->geometry().height();
                if(desktopHeight < sum)
                    desktopHeight = sum;
            }

            xcb_ewmh_wm_strut_partial_t strut;
            memset(&strut, 0, sizeof(xcb_ewmh_wm_strut_partial_t));
            if(bottom)
            {
                int bottom = panel->height() + desktopHeight - (screen.height() + screen.y());
                xcb_ewmh_set_wm_strut(&ewmh, panel->winId(), 0, 0, 0, bottom);
                strut.bottom = bottom;
                strut.bottom_start_x = screen.x();
                strut.bottom_end_x = screen.x() + screen.width() - 1;
            }
            else
            {
                int top = panel->height() + screen.y();
                xcb_ewmh_set_wm_strut(&ewmh, panel->winId(), 0, 0, top, 0);
                strut.top = top;
                strut.top_start_x = screen.x();
                strut.top_end_x = screen.x() + screen.width() - 1;
            }
            xcb_ewmh_set_wm_strut_partial(&ewmh, panel->winId(), strut);
            xcb_ewmh_set_wm_desktop(&ewmh, panel->winId(), 0xFFFFFFFF);
            xcb_flush(conn);
            xcb_ewmh_connection_wipe(&ewmh);

            panel->show(); // Necessary here for borders and under\overline to work
            panel->setObjectName(name);
            widgets.insert(std::make_pair(name, panel));

            setProperties(panel);

            if(config->contains("texture"))
            {
                auto texturePath = config->value("texture").toString();
                if(QDir::isRelativePath(texturePath))
                    texturePath.prepend(configDir());
                panel->setTexture(texturePath);
            }

            panel->setSpacing('L', config->value("spacing-left", 5).toInt());
            panel->setSpacing('C', config->value("spacing-center", 5).toInt());
            panel->setSpacing('R', config->value("spacing-right", 5).toInt());
            panel->setPadding(config->value("inner-margins", 0).toInt());

            for(const auto &type:QStringList{"modules-left", "modules-center", "modules-right"})
                for(const auto &m:config->value(type).toStringList())
                    if(widgets[m] != nullptr)
                    {
                        auto location = type[8]; // Kinda cheating, 9th character could be L, C or R
                        widgets[m]->setParent(panel);
                        panel->addModule(location.toLatin1(), widgets[m]);
                    }

            config->endGroup();
        }
    }
}
