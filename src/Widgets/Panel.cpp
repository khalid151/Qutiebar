#include "Widgets/Panel.h"

namespace Widgets
{
    Panel::Panel(const QRect &geometry, int panelWidth, int panelHeight,
            int margin, int padding) : Utils::WidgetProperties(this, false)
    {
        event = new Utils::EventHandler;
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        screenHeight = geometry.height();
        screenWidth = geometry.width();
        this->panelHeight = panelHeight;
        this->panelWidth = panelWidth;
        this->padding = padding;

        // Setting layouts and stuff
        background = new QWidget(this);
        mainWidget = new QWidget(background);
        mainWidget->setLayout(&mainLayout);
        leftLayout.setContentsMargins(0, 0, 0, 0);
        centerLayout.setContentsMargins(0, 0, 0, 0);
        rightLayout.setContentsMargins(0, 0, 0, 0);
        mainLayout.addLayout(&leftLayout, 1);
        mainLayout.addLayout(&centerLayout, 1);
        mainLayout.addLayout(&rightLayout, 1);
        mainLayout.setAlignment(Qt::AlignHCenter);
        leftLayout.setAlignment(Qt::AlignLeft);
        centerLayout.setAlignment(Qt::AlignHCenter);
        rightLayout.setAlignment(Qt::AlignRight);

        // Setting panel
        setWindowFlag(Qt::WindowDoesNotAcceptFocus);
        setAttribute(Qt::WA_TranslucentBackground);
        setContentsMargins(0, 0, 0, 0);
        setMargins(margin);
        this->setMinimumWidth(screenWidth);
        mainWidget->setMaximumWidth(screenWidth - 2 * margin);

        // So child widgets won't inherit background properties
        mainWidget->setStyleSheet("border: none; background: transparent; border-radius: 0;");
    }

    void
    Panel::setBackground(const QColor &c)
    {
        QString color = QColorToString(c);
        background->setStyleSheet(QString("background-color: %1").arg(color));
    }

    void
    Panel::setTexture(const QString &location)
    {
        // Pixmap as a background, won't work if a background color is set
        QPixmap pixmap(location);
        pixmap = pixmap.scaledToHeight(this->height());
        QPalette palette;
        palette.setBrush(QPalette::Background, pixmap);
        background->setAutoFillBackground(true);
        background->setPalette(palette);
    }

    void
    Panel::setBorder(int width, const QColor &c)
    {
        if(width > 0) {
            QString color = QColorToString(c);
            setStyleSheet(QString("border: %1px solid %2").arg(
                        QString::number(width), color));
        } else {
            setStyleSheet("border: none");
        }
    }

    void
    Panel::setRoundCorners(int radius)
    {
        background->setStyleSheet(QString("%1; border-style: solid;\
                                border-radius: %2px").arg(background->styleSheet(),
                                QString::number(radius)));
    }

    void
    Panel::addModule(const char location, QWidget *widget)
    {
        switch(location)
        {
            case 'l': case 'L':
                leftLayout.addWidget(widget);
                break;
            case 'c': case 'C':
                centerLayout.addWidget(widget);
                break;
            case 'r': case 'R':
                rightLayout.addWidget(widget);
                break;
            default:
                break;
        }
    }

    void
    Panel::setSpacing(const char location, int spacing)
    {
        switch(location)
        {
            case 'l': case 'L':
                leftLayout.setSpacing(spacing);
                break;
            case 'c': case 'C':
                centerLayout.setSpacing(spacing);
                break;
            case 'r': case 'R':
                rightLayout.setSpacing(spacing);
                break;
            default:
                break;
        }
    }

    void
    Panel::setPadding(int p)
    {
        // Confusing names.. Well, this adds a margin for modules\widgets | padding for panel
        mainWidget->setContentsMargins(p, p, p, p);
    }
    void
    Panel::setMargins(int margin)
    {
        // Will make the panel "float"
        resize(screenWidth - 2*margin, panelHeight + 2*margin);
        mainWidget->resize(size());
        background->setGeometry(QRect((screenWidth - panelWidth)/2+margin,
                    margin, panelWidth - 2*margin, height() - 2*margin));
        mainLayout.setContentsMargins(padding, 0, screenWidth - panelWidth
                + padding, 2*margin);
    }
}
