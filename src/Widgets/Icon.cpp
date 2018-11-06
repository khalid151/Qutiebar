#include <utility>
#include <QFileInfo>
#include <QPainter>

#include "Widgets/Icon.h"

namespace Widgets
{
    PixIcon::PixIcon(int w, int h, const QString &location, QWidget *parent)
        : QWidget(parent)
    {
        height = h;
        width = w;
        if(!location.isEmpty())
        {
            load(location);
        }
        resize(h, w);
    }

    void
    PixIcon::setAA(bool enable)
    {
        aa = enable;
    }

    void
    PixIcon::setSmoothing(bool enable)
    {
        smooth = enable;
    }

    void
    PixIcon::load(const QString &p)
    {
        loadPixmap = true;
        if(QFileInfo::exists(p))
            icon.load(p);
        else
        {
            icon = QIcon::fromTheme(p).pixmap(width, height);
            update();
        }
        storedIcon = p;
    }

    void
    PixIcon::load(const QIcon &icon)
    {
        loadPixmap = false;
        qIcon = icon;
        this->icon = icon.pixmap(width, height);
        update();
    }

    void
    PixIcon::resize(int w, int h)
    {
        height = h;
        width = w;
        setMinimumSize(h, w);
        setMaximumSize(h, w);
        if(loadPixmap) load(storedIcon);
        else load(qIcon);
    }

    void
    PixIcon::paintEvent(QPaintEvent *event)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, smooth);
        painter.setRenderHint(QPainter::Antialiasing, aa);
        painter.drawPixmap(0, 0, height, width, icon);
        QWidget::paintEvent(event);
    }
    /* -------------------------------------------------------- */
    Icon::Icon(const QString &location, int w, int h, int padding, QWidget *parent)
        : QWidget(parent), Utils::WidgetProperties(this)
    {
        type = IconType::Pixmap;
        this->padding = padding;

        event = new Utils::EventHandler;
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        setLayout(&layoutContainer);
        layoutContainer.setContentsMargins(padding, 0, padding, 0);

        icon = std::make_unique<PixIcon>(w, h, location, this);
        layoutContainer.addWidget(icon.get());
        icon->setStyleSheet("border: none; background: none");
        resize(h, w);
    }

    Icon::Icon(const QString &icn, int pointSize, int padding, QWidget *parent)
        : QWidget(parent), WidgetProperties(this)
    {
        type = IconType::Text;
        this->padding = padding;

        event = new Utils::EventHandler;
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        setLayout(&layoutContainer);
        layoutContainer.setContentsMargins(padding, 0, padding, 0);

        icon = std::make_unique<Text>(icn, padding, this);
        layoutContainer.addWidget(icon.get());
        icon->setStyleSheet("border: none; background: none");
        resize(pointSize);

        icon->removeEventFilter(dynamic_cast<Text*>(icon.get())->event);
    }

    Icon::~Icon()
    {
        delete event;
    }

    void
    Icon::setAntialiasing(bool aa, bool smoothing)
    {
        if(icon == nullptr)
            return;
        dynamic_cast<PixIcon*>(icon.get())->setAA(aa);
        dynamic_cast<PixIcon*>(icon.get())->setSmoothing(smoothing);
    }

    void
    Icon::load(const QString &newIcon)
    {
        switch(type)
        {
            case IconType::Pixmap:
                dynamic_cast<PixIcon*>(icon.get())->load(newIcon);
                break;
            case IconType::Text:
                dynamic_cast<Text*>(icon.get())->setText(newIcon);
                break;
        }
    }

    void
    Icon::setIcon(const QIcon &i)
    {
        dynamic_cast<PixIcon*>(icon.get())->load(std::move(i));
    }

    void
    Icon::resize(int pointSize)
    {
        dynamic_cast<Text*>(icon.get())->setFontSize(pointSize);
    }

    void
    Icon::resize(int w, int h)
    {
        dynamic_cast<PixIcon*>(icon.get())->resize(w, h);
    }
}
