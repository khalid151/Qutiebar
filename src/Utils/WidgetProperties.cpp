#include <QGraphicsDropShadowEffect>

#include "Utils/WidgetProperties.h"

namespace Utils
{
    WidgetProperties::WidgetProperties(QWidget *widget, bool clear)
    {
        if(clear) // To "clear" properties if widgets were nested
            widget->setStyleSheet("border: none; background: none; border-radius: none");
        this->w = widget;
    }

    void
    WidgetProperties::setVerticalAlignment(int inc)
    {
        vAlign = inc;
        if (inc > 0) {
            w->setContentsMargins(this->padding, 0, this->padding, inc);
        } else {
            w->setContentsMargins(this->padding, inc * -1, this->padding, 0);
        }
    }

    void
    WidgetProperties::setForeground(const QColor &c)
    {
        QString oldStyleSheet = w->styleSheet();
        QString color = QColorToString(c);
        w->setStyleSheet(QString("%1; color: %2").arg(oldStyleSheet, color));
    }

    void
    WidgetProperties::setBackground(const QColor &c)
    {
        QString oldStyleSheet = w->styleSheet();
        QString color = QColorToString(c);
        w->setStyleSheet(QString("%1; background-color: %2").arg(oldStyleSheet, color));
    }

    void
    WidgetProperties::setShadow(int r, int x, int y, const QColor &c)
    {
        auto *eff = new QGraphicsDropShadowEffect();
        eff->setBlurRadius(r);
        eff->setXOffset(x);
        eff->setYOffset(y);
        eff->setColor(c);
        w->setGraphicsEffect(eff);
    }

    void
    WidgetProperties::setBorder(int width, const QColor &c)
    {
        QString oldStyleSheet = w->styleSheet();
        if(width > 0) {
            QString color = QColorToString(c);
            w->setStyleSheet(QString("%1; border: %2px solid %3").arg(
                        oldStyleSheet, QString::number(width), color));
        } else {
            w->setStyleSheet("border: none");
        }
    }

    void
    WidgetProperties::setUnderline(int width, const QColor &c)
    {
        QString oldStyleSheet = w->styleSheet();
        if(width > 0) {
            QString color = QColorToString(c);
            w->setStyleSheet(QString("%1; border-bottom: %2px solid %3").arg(
                        oldStyleSheet, QString::number(width), color));
        } else {
            w->setStyleSheet(QString("%1; border-bottom: none").arg(
                        oldStyleSheet));
        }
    }

    void
    WidgetProperties::setOverline(int width, const QColor &c)
    {
        QString oldStyleSheet = w->styleSheet();
        if(width > 0) {
            QString color = QColorToString(c);
            w->setStyleSheet(QString("%1; border-top: %2px solid %3").arg(
                        oldStyleSheet, QString::number(width), color));
        } else {
            w->setStyleSheet(QString("%1; border-top: none").arg(
                        oldStyleSheet));
        }
    }


    void
    WidgetProperties::setFontFamily(const QString& font)
    {
        setFontFamily(QStringList(font));
    }

    void
    WidgetProperties::setFontFamily(const QStringList& fontsList)
    {
        QString oldStyleSheet = w->styleSheet();
        w->setStyleSheet(QString("%1; font-family: %2;").arg(oldStyleSheet, fontsList.join(",")));
    }

    void
    WidgetProperties::setFontSize(int pointSize)
    {
        QFont font = w->font();
        font.setPointSize(pointSize);
        w->setFont(font);
    }

    void
    WidgetProperties::setRoundCorners(int radius)
    {
        QString stylesheet = w->styleSheet();
        w->setStyleSheet(QString("border-style: solid;\
                                border-radius: %1px;\
                                %2;").arg(QString::number(radius), stylesheet));
    }

    void
    WidgetProperties::setPadding(int padding)
    {
        this->padding = padding;
        setVerticalAlignment(vAlign);
    }

    // Private
    QString
    WidgetProperties::QColorToString(const QColor &c)
    {
        QString color = QString("rgba(%1, %2, %3, %4)").arg(
                QString::number(c.red()),
                QString::number(c.green()),
                QString::number(c.blue()),
                QString::number(static_cast<float>(c.alpha()/255.0)));

        return color;
    }
}
