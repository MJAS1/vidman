#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>

class TextEdit;

//Code from Qt Examples
class LineNumberArea : public QWidget
{
    Q_OBJECT
public:
    explicit LineNumberArea(TextEdit *editor = 0);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent * ev);

private:
    TextEdit* textEdit;
};

#endif // LINENUMBERAREA_H
