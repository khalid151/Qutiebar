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
        auto font = config->contains("progress-font-color") ?
            getConfiguredColor("progress-font-color") : filled;
        int width = config->value("progress-width", 2).toInt();
        if(config->contains("progress-empty-width"))
        {
            int emptyWidth = config->value("progress-empty-width").toInt();
            p->setStyle(width, filled, empty, font, emptyWidth);
        }
        else
            p->setStyle(width, filled, empty, font);

        auto valign = config->value("progress-vertical-align", 0).toInt();
        p->setVerticalAlignment(valign);

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

        if(config->contains("shadow") || config->contains("shadow-radius"))
        {
            auto c = config->contains("shadow") ?
                getConfiguredColor("shadow") : Qt::black;
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

    // Building functions
    std::unique_ptr<Modules::DisplayItem>
    Builder::buildDisplayItem(Utils::DataModel *data, int w, int h, int p)
    {
        // Creating modules based on changing percentage (0 - 100)
        // using DataModel (Utils/Misc.h) to get said percentage.
        using namespace Modules;

        auto update = config->value("update-rate", 1).toFloat();
        DisplayItem::TextLocation tl;
        if(config->contains("text-location"))
        {
            if(config->value("text-location").toString() == "left")
                tl = DisplayItem::LEFT;
            else if(config->value("text-location").toString() == "right")
                tl = DisplayItem::RIGHT;
            else
                tl = DisplayItem::NONE;
        }
        else tl = DisplayItem::NONE;

        // type: 0 -> TEXT, 1 -> FONTICON, 2 -> PIXICON, 3 -> CIRCLE, 4 -> TEXTCIRCLE
        auto type = static_cast<DisplayItem::Type>(config->value("type", 0).toInt());
        auto displayItem = std::make_unique<DisplayItem>(type, tl, w, h, p, update * 1000);
        displayItem->setData(data);

        setProperties(displayItem.get());
        configureProgress(displayItem->progress());

        if(config->contains("icon-color"))
            displayItem->enableIconColorChange(false);
        else
            displayItem->enableIconColorChange();

        auto primaryIcons = config->value("icons").toStringList();
        auto secondaryIcons = config->value("secondary-icons").toStringList();
        if(!secondaryIcons.isEmpty())
        {
            displayItem->setIcons(primaryIcons, secondaryIcons);
        } else
            displayItem->setIcons(primaryIcons);

        if(type == DisplayItem::TEXTCIRCLE || type == DisplayItem::CIRCLE)
        {
            if(config->contains("foreground"))
            {
                auto color = getConfiguredColor("foreground");
                displayItem->progress()->setFontColor(color);
            }
            if(config->contains("font-size"))
                displayItem->progress()->setFontSize(config->value("font-size").toInt());
        }

        if(config->contains("font-size") && displayItem->percentage() != nullptr)
            displayItem->percentage()->setFontSize(config->value("font-size").toInt());
        if(config->contains("icon-size") && displayItem->icon() != nullptr)
            displayItem->icon()->resize(config->value("icon-size").toInt());

        if(config->contains("percentage-color") && displayItem->percentage() != nullptr)
        {
            auto color = getConfiguredColor("percentage-color");
            displayItem->percentage()->setForeground(color);
        }
        if(config->contains("icon-color") && displayItem->icon() != nullptr)
        {
            auto color = getConfiguredColor("icon-color");
            displayItem->icon()->setForeground(color);
        }
        if(config->contains("percentage-background") && displayItem->percentage() != nullptr)
        {
            auto color = getConfiguredColor("percentage-background");
            displayItem->percentage()->setBackground(color);
        }
        if(config->contains("icon-background") && displayItem->icon() != nullptr)
        {
            auto color = getConfiguredColor("icon-background");
            displayItem->icon()->setBackground(color);
        }

        return displayItem;
    }

    void
    Builder::buildModules()
    {
        auto modules = generateModulesList(); // Just what panels need

        auto hasModule = [this](auto n) {
            auto keys = config->allKeys();
            QRegularExpression regex(QString(".*/%1/.*").arg(n));
            return !keys.filter(regex).isEmpty();
        };

        for(const auto &name:modules)
        {
            if(!hasModule(name))
                continue; // Skip if it doesn't have the module

            auto type = name.left(name.indexOf("-"));
            config->beginGroup(QString("module/%1").arg(name));

            int width = config->value("width", 25).toInt();
            int height = config->value("height", 25).toInt();
            int padding = config->value("padding", 0).toInt();

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
                wm_class ? xevents->trackClass() : xevents->trackTitle();
                QObject::connect(xevents.get(), wm_class ?
                        &Utils::X11EventHandler::classChanged : &Utils::X11EventHandler::titleChanged,
                        title.get(), &Widgets::Text::updateText);

                if(config->contains("max-length"))
                {
                    int maxlen = config->value("max-length").toInt();
                    float speed = config->value("scroll-speed", 0.25).toFloat();
                    title->setMaxLength(maxlen, speed);
                }

                if(config->value("rich-text", false).toBool())
                    title->enableRichText();

                widgets.insert(std::make_pair(name, title.get()));
                moduleList.push_back(std::move(title));
            }
            else if(type == "clock")
            {
                auto update = config->value("update-rate", 1).toFloat();
                auto format = config->value("format", "hh:mm a").toString();
                auto clock = std::make_unique<Modules::Clock>(padding, format, update * 1000);
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
                xevents->trackWorkspace();
                Widgets::Icon::IconType ic;
                if(config->value("pixmap", false).toBool()) ic = Widgets::Icon::PIXMAP;
                else ic = Widgets::Icon::TEXT;
                // Current -> only one workspace displayed at a time
                auto t = config->value("current-only", false).toBool()? Modules::Desktops::CURRENT
                    : Modules::Desktops::POPULATED;

                auto iconList = config->value("icons").toStringList();
                if(config->contains("default-icon"))
                    iconList.append(QString("default:%1").arg(config->value("default-icon").toString()));

                auto desktops = std::make_unique<Modules::Desktops>(t, ic, iconList, padding);

                auto aa = config->value("antialiasing", true).toBool();
                setProperties(desktops.get());
                if(ic == Widgets::Icon::PIXMAP)
                {
                    desktops->resize(width, height);
                    desktops->setAntialiasing(aa, aa);
                }

                desktops->setSpacing(config->value("spacing", 5).toInt());

                // Setting indicators
                Modules::Desktops::Lines l;
                QColor foreground = Qt::white, background = Qt::transparent, line = Qt::red;
                if(config->contains("active-foreground"))
                    foreground = getConfiguredColor("active-foreground");
                if(config->contains("active-background"))
                    background = getConfiguredColor("active-background");

                if(config->contains("active-underline-color")) {
                    line = getConfiguredColor("active-underline-color");
                    l= Modules::Desktops::UNDERLINE;
                } else if(config->contains("active-overline-color")) {
                    line = getConfiguredColor("active-overline-color");
                    l= Modules::Desktops::OVERLINE;
                } else
                    l= Modules::Desktops::NONE;

                desktops->setIndicatorStyle(Modules::Desktops::ACTIVE, foreground, background,
                        line, config->value("active-line-width", 0).toInt(), l);

                foreground = config->contains("inactive-foreground") ?
                    getConfiguredColor("inactive-foreground") : foreground;
                background = config->contains("inactive-background") ?
                    getConfiguredColor("inactive-background") : Qt::transparent;

                if(config->contains("inactive-underline-color")) {
                    line = getConfiguredColor("inactive-underline-color");
                    l= Modules::Desktops::UNDERLINE;
                } else if(config->contains("inactive-overline-color")) {
                    line = getConfiguredColor("inactive-overline-color");
                    l= Modules::Desktops::OVERLINE;
                } else
                    l= Modules::Desktops::NONE;

                if(config->contains("font-size"))
                    desktops->resize(config->value("font-size").toInt());

                desktops->setIndicatorStyle(Modules::Desktops::INACTIVE, foreground, background,
                        line, config->value("inactive-line-width", 0).toInt(), l);

                QObject::connect(xevents.get(), &Utils::X11EventHandler::desktopChanged,
                        desktops.get(), &Modules::Desktops::updateDesktops);

                widgets.insert(std::make_pair(name, desktops.get()));
                moduleList.push_back(std::move(desktops));
            }
            else if(type == "battery")
            {
                auto battery = QString("/sys/class/power_supply/%1").arg(config->value("battery", "BAT0").toString());
                auto batteryData = std::make_unique<Data::Battery>(battery);
                auto batteryItem = buildDisplayItem(batteryData.get(), width, height, padding);

                auto charging = config->contains("charging-color") ?
                    getConfiguredColor("charging-color") : Qt::green;
                auto discharging = config->contains("discharging-color") ?
                    getConfiguredColor("discharging-color") : Qt::black;

                if(config->contains("charging-icons"))
                {
                    auto icons = config->value("charging-icons").toStringList();
                    if(config->contains("discharging-icons"))
                        batteryItem->setIcons(icons, config->value("discharging-icons").toStringList());
                    else
                        batteryItem->setIcons(icons, icons);
                }

                batteryItem->setColors(charging, discharging);

                widgets.insert(std::make_pair(name, batteryItem.get()));
                moduleList.push_back(std::move(batteryItem));
                dataList.push_back(std::move(batteryData));
            }
#ifdef ENABLE_ALSA
            else if(type == "volume")
            {
                auto soundcard = config->value("soundcard", "default").toString();
                auto mixer = config->value("mixer", "Master").toString();
                auto volumeData = std::make_unique<Data::Volume>(soundcard, mixer);
                auto volumeItem = buildDisplayItem(volumeData.get(), width, height, padding);

                widgets.insert(std::make_pair(name, volumeItem.get()));
                moduleList.push_back(std::move(volumeItem));
                dataList.push_back(std::move(volumeData));
            }
#endif
            else if(type == "backlight")
            {
                auto backlight = config->value("backlight", "intel_backlight").toString();
                auto backlightData = std::make_unique<Data::Backlight>(backlight.prepend("/sys/class/backlight/"));
                auto backlightItem = buildDisplayItem(backlightData.get(), width, height, padding);

                widgets.insert(std::make_pair(name, backlightItem.get()));
                moduleList.push_back(std::move(backlightItem));
                dataList.push_back(std::move(backlightData));
            }
#if defined(ENABLE_MPD) || defined(ENABLE_DBUS)
            else if(type == "mpd" || type == "mpris")
            {
                std::unique_ptr<Modules::MusicDisplay> music;
                auto update = config->value("update-rate", 1).toFloat();

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
                auto len = config->value("progress-length", 20).toInt();
                auto valign = config->value("progress-vertical-align", 0).toInt();
                auto iconSize = config->value("icon-size", 10).toInt();

                QStringList defaultOrder{"text", "buttons", "progress"};
                auto order = config->value("order", defaultOrder).toStringList();

                for(const auto &o:order) {
                    if(o == "text")
                    {
                        if(config->contains("text-format"))
                        music->addText(textFmt, maxlen, padding);
                    }
                    else if(o == "buttons")
                    {
                        if(config->contains("add-buttons"))
                        {
                            bool aa = config->value("antialiasing", true).toBool();
                            auto type = config->value("button-type", "text").toString();
                            if(type == "icon")
                                music->addButtons(width, height, aa, buttons);
                            else
                                music->addButtons(iconSize, buttons);
                            auto props = std::make_unique<Utils::WidgetProperties>(music->getButtons());
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
                            configureProgress(music->getProgress());
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
        auto hasModule = [this](auto t, auto n) {   // To check if a custom module exists
            auto keys = config->allKeys();
            QRegularExpression regex(QString("%1/%2/.*").arg(t, n));
            return !keys.filter(regex).isEmpty();
        };

        QStringList types{"text", "progress", "icon", "percentage", "script"};
        for(const auto &type:types)
        {
            if(hasModule(type, name))
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
                else if(type == "progress")
                {
                    std::unique_ptr<Progress> progress;

                    auto readStdOut = [](auto p, auto w) {
                        int perc = p->readAll().simplified().toInt();
                        w->updateProgress(perc);
                    };
                    int percentage = config->value("starting-percentage", 0).toInt();

                    if(config->contains("progress-radius"))
                    {
                        int r = config->value("progress-radius").toInt();
                        bool showPercentage = config->value("show-percentage", false).toBool();
                        progress = std::make_unique<Progress>(r, showPercentage, padding);
                    }
                    else if(config->contains("progress-length"))
                    {
                        int l = config->value("progress-length").toInt();
                        progress = std::make_unique<Progress>(l, padding);
                    }
                    else
                    {
                        config->endGroup();
                        continue; // Skip module if no progress is specified.
                    }

                    progress->updateProgress(percentage);
                    events = progress->event;
                    setProperties(progress.get());
                    configureProgress(progress.get());
                    QObject::connect(proc.get(), &QProcess::readyReadStandardOutput,
                            progress.get(), std::bind(readStdOut, proc.get(), progress.get()));
                    widgets.insert(std::make_pair(name, progress.get()));
                    moduleList.push_back(std::move(progress));
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
                else if(type == "percentage")
                {
                    auto data = std::make_unique<Data::Process>(proc.get());
                    auto display = buildDisplayItem(data.get(), width, height, padding);
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

            panelList.push_back(std::make_unique<Widgets::Panel>(screen, width, height, margins, padding));
            auto panel = panelList.back().get();
            bool over = config->value("over-windows", true).toBool();
            bool bottom = config->value("bottom", false).toBool();
            if(bottom) panel->move(screen.x(), screen.y() + screen.height() - panel->height());
            else panel->move(screen.x(), screen.y());

            // Setting EWMH hints
            auto conn = QX11Info::connection();
            xcb_ewmh_connection_t *ewmh;
            ewmh = new xcb_ewmh_connection_t;
            xcb_ewmh_init_atoms_replies(ewmh, xcb_ewmh_init_atoms(conn, ewmh), nullptr);

            xcb_atom_t states[2] = {ewmh->_NET_WM_STATE_STICKY};
            if(over) states[1] = ewmh->_NET_WM_STATE_ABOVE;
            else states[1] = ewmh->_NET_WM_STATE_BELOW;
            xcb_atom_t type[] = {ewmh->_NET_WM_WINDOW_TYPE_DOCK};
            xcb_ewmh_set_wm_state(ewmh, panel->winId(), 2, states);
            xcb_ewmh_set_wm_window_type(ewmh, panel->winId(), 1, type);

            // Get total width and height of desktop
            uint32_t dw, dh;
            xcb_ewmh_get_desktop_geometry_reply(ewmh, xcb_ewmh_get_desktop_geometry(ewmh, 0), &dw, &dh, nullptr);

            xcb_ewmh_wm_strut_partial_t strut;
            memset(&strut, 0, sizeof(xcb_ewmh_wm_strut_partial_t));
            if(bottom)
            {
                int bottom = panel->height() + dh - (screen.height() + screen.y());
                xcb_ewmh_set_wm_strut(ewmh, panel->winId(), 0, 0, 0, bottom);
                strut.bottom = bottom;
                strut.bottom_start_x = screen.x();
                strut.bottom_end_x = screen.x() + screen.width() - 1;
            }
            else
            {
                int top = panel->height() + screen.y();
                xcb_ewmh_set_wm_strut(ewmh, panel->winId(), 0, 0, top, 0);
                strut.top = top;
                strut.top_start_x = screen.x();
                strut.top_end_x = screen.x() + screen.width() - 1;
            }
            xcb_ewmh_set_wm_strut_partial(ewmh, panel->winId(), strut);
            xcb_ewmh_set_wm_desktop(ewmh, panel->winId(), 0xFFFFFFFF);
            xcb_flush(conn);
            xcb_ewmh_connection_wipe(ewmh);

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
