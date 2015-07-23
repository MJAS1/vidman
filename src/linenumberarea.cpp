#include "linenumberarea.h"
#include "textedit.h"

LineNumberArea::LineNumberArea(TextEdit *editor) :
    QWidget(editor)
{
    textEdit = editor;
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(textEdit->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *ev)
{
    textEdit->lineNumberAreaPaintEvent(ev);
}
