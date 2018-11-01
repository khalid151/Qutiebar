#include <QRegularExpression>
#include "Utils/Misc.h"

namespace Utils
{
    QStringList arguments(QString s)
    {
        QStringList args;
        while(!s.isEmpty())
        {
            if(s.startsWith('"'))
            {
                args.append(s.mid(1, s.lastIndexOf('"') -1));
                s.remove(s.left(s.lastIndexOf('"') + 1));
            }
            else
            {
                int index = s.indexOf(' ') + 1;
                if(!index) {
                    args.append(s);
                    break;
                }
                args.append(s.left(index));
                s.remove(args.back());
                args.back().remove(' ');
            }
        }
        return args;
    }

    QList<char> tags{'a', 'b', 'c', 'd', 'e', 'f', 'h', 'i', 'l', 'o', 'p', 's', '/'};

    QString section(const QString &s, int start, int end)
    {
        QString str;
        int count = 0;
        int raw_count = -1;
        bool tag = false;
        for(auto c:s)
        {
            raw_count++;
            if(!tag)
            {
                if(c == '<' && tags.contains(s[raw_count+1].toLatin1()))
                {
                    str.append(c);
                    tag = true;
                    continue;
                }
                count++;
                if(count >= start && count <= end) str.append(c);
            }
            else
            {
                if(c == '>') tag = false;
                str.append(c);
                continue;
            }
        }
        return str;
    }

    int rawStrLength(const QString &s)
    {
        int count = 0;
        int raw_count = -1;
        bool tag = false;
        for(auto c:s)
        {
            raw_count++;
            if(!tag)
            {
                if(c == '<' && tags.contains(s[raw_count+1].toLatin1()))
                {
                    tag = true;
                    continue;
                }
                count++;
            }
            else
            {
                if(c == '>') tag = false;
                continue;
            }

        }
        return count;
    }

    QColor getColor(const QString &c)
    {
        QColor color(c);
        if(!color.isValid())
        {
            QRegularExpression
            re("(?<r>\\d+).*?(?<g>\\d+).*?(?<b>\\d+).*?(?<a>\\d*\\.?\\d+)");
            auto m = re.match(c);
            auto alpha = m.captured("a").toFloat() * 255;
            alpha = alpha > 255 ? 255 : alpha;
            color = QColor(m.captured("r").toInt(),
                    m.captured("g").toInt(),
                    m.captured("b").toInt(),
                    static_cast<int>(alpha));
            if(!color.isValid()) return Qt::transparent;
            else return color;
        } else
            return color;
    }
}
