#include "Widgets/Text.h"
#include "Utils/Misc.h"

namespace Widgets
{
    Text::Text(const QString &text, int padding, QWidget *parent)
        : QLabel(text, parent), Utils::WidgetProperties(this)
    {
        event = new Utils::EventHandler(this);
        setAttribute(Qt::WA_Hover);
        installEventFilter(event);

        setTextFormat(Qt::PlainText);

        setAlignment(Qt::AlignHCenter | Qt::AlignCenter);
        this->padding = padding;
        setContentsMargins(padding, 0, padding, 0);
    }

    Text::~Text()
    {
        if(textScroller != nullptr)
            textScroller->stop();
    }

    void
    Text::enableRichText()
    {
        setTextFormat(Qt::AutoText); // Let it decide
    }

    void
    Text::setMaxLength(const int length, const float scrollSpeed)
    {
        // Setting max length will enable text scrolling
        maxLengthSet = true;
        maxLength = length;
        bool richText = textFormat() != Qt::PlainText;
        textScroller = new Utils::Scroller(richText, this);
        textScroller->setScrollSpeed(scrollSpeed);
        QString width;
        QFontMetrics fm(this->font());
        this->setMaximumWidth(fm.width(width.fill('x', length + 2)));
        connect(textScroller, &Utils::Scroller::sendText, this, &Text::setText);
    }

    void
    Text::updateText(const QString &text)
    {
        int len = 0;
        if(textFormat() == Qt::PlainText) len = text.size();
        else len = Utils::rawStrLength(text); // Length without tags (if it was formatted)

        if(!maxLengthSet)
        {
            setText(text);
        }
        else if(maxLengthSet && len > maxLength)
        {
            textScroller->stop(); // So you can start a new title fresh
            textScroller->wait();
            textScroller->setText(text, maxLength);
            if(textScroller != nullptr) textScroller->start();
        }
        else if(maxLengthSet && len < maxLength)
        {
            textScroller->stop();
            setText(text);
        }
    }
}
