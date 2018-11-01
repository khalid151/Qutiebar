#ifndef WIDGETPROPS_H
#define WIDGETPROPS_H

#include <QObject>
#include <QWidget>
#include <QFont>

namespace Utils
{
    class WidgetProperties
    {
        public:
            WidgetProperties(QWidget*, bool = true);

            void setVerticalAlignment(int);
            void setForeground(const QColor&);
            void setBackground(const QColor&);
            void setShadow(int r = 10, int x = 0, int y = 0, const QColor& = Qt::black);
            void setBorder(int, const QColor&);
            void setUnderline(int, const QColor&);
            void setOverline(int, const QColor&);
            void setFontFamily(const QString&);
            void setFontFamily(const QStringList&);
            void setFontSize(int);
            void setRoundCorners(int r=10);
            void setPadding(int);

        protected:
            QString QColorToString(const QColor&);
            int padding = 0;
            int vAlign = 0;

        private:
            QWidget *w;
    };
}

#endif // WIDGETPROPS_H
