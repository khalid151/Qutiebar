#include <QPainter>
#include <QFontMetrics>

#include "Widgets/Progress.h"

namespace Widgets
{
    Progress::Progress(int r, bool showPercent, int p, QWidget *parent)
        : QWidget(parent), Utils::WidgetProperties(this)
    {
        event = new Utils::EventHandler(this);
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        padding = p;

        this->showPercent = showPercent;
        type = 'C';
        shapeHeight = r;
        shapeWidth = r;
        resize(r + (p*2), r); // Add padding as widget size
    }

    Progress::Progress(int w, int p, QWidget *parent)
        : QWidget(parent), Utils::WidgetProperties(this)
    {
        event = new Utils::EventHandler(this);
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        padding = p;

        type = 'L';
        shapeHeight = w;
        shapeWidth = 2;
        resize(w + (p*2), 2);
    }

    void
    Progress::setColor(const QColor &c)
    {
        primaryColor = c;
    }

    void
    Progress::setFontColor(const QColor &c)
    {
        fontColor = c;
    }

    void
    Progress::setStyle(int w, const QColor &c1, const QColor &c2,
            const QColor &c3, int pen2Width)
    {
        penWidth = w;
        primaryColor = c1;
        secondaryColor = c2;
        fontColor = c3;
        if (pen2Width) {
            secondPenWidth = pen2Width;
        } else {
            secondPenWidth = penWidth - 1;
        }
    }

    void
    Progress::setStyle(int w, const QColor &c1, const QColor &c2)
    {
        penWidth = w;
        primaryColor = c1;
        secondaryColor = c2;
        secondPenWidth = penWidth - 1;
    }

    void
    Progress::setVerticalAlignment(int v)
    {
        valign = v * -1; // To keep it consistent with other widgets
    }

    void
    Progress::updateProgress(int p)
    {
        if(p >= 0 && p <= 100)
            progress = p;
        update(); // Redraw after progress change
    }

    void
    Progress::setCircleIcon(const QString &icon)
    {
        showPercent = false;
        showIcon = true;
        this->icon = icon;
    }

    void
    Progress::resize(int w, int h)
    {
        setMinimumSize(w, h);
    }

    void
    Progress::paintEvent(QPaintEvent *event)
    {
        int degree = static_cast<int>(3.6 * progress); // For circle
        int steps = static_cast<int>((minimumWidth()/100.0) * progress); // For line
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        QPen foreground(primaryColor, penWidth, Qt::SolidLine);
        QPen background(secondaryColor, secondPenWidth, Qt::SolidLine);

        switch(type)
        {
            case 'C':
                painter.setPen(background);
                painter.drawEllipse(width()/2 - shapeWidth/2 + penWidth/2,
                        height()/2 - shapeHeight/2 + penWidth/2,
                        shapeHeight - penWidth, shapeWidth - penWidth);
                painter.setPen(foreground);
                painter.drawArc(width()/2 - shapeWidth/2 + penWidth/2,
                        height()/2 - shapeHeight/2 + penWidth/2 + valign,
                         shapeHeight - penWidth, shapeWidth - penWidth, 16*90,
                        -16*degree);
                if((showPercent && progress < 100) || showIcon)
                {
                    QPen color(fontColor);
                    QFontMetrics fm(font());
                    painter.setPen(color);
                    QString toDraw;
                    if(showPercent) toDraw = QString::number(progress);
                    else if(showIcon) toDraw = icon;
                    int fontWidth = fm.width(toDraw);
                    int fontHeight = fm.height();
                    painter.drawText(width()/2 - fontWidth/2,
                            height()/2 + fontHeight/3 + valign, toDraw);
                }
                break;

            case 'L':
                painter.setPen(background);
                painter.drawLine(0, height()/2 + valign, width(), height()/2 + valign);
                painter.setPen(foreground);
                painter.drawLine(0, height()/2 + valign, steps, height()/2 + valign);
                break;
        }
        QWidget::paintEvent(event);
    }
}
