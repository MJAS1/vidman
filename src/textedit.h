#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QPlainTextEdit>

/*!
 * Text editor with line numbers in the margin and the current line highlighted.
 */
class TextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = 0);
    
    void    lineNumberAreaPaintEvent(QPaintEvent *ev);
    int     lineNumberAreaWidth();
    
protected:
    void    resizeEvent(QResizeEvent *ev);

private slots:
    void    updateLineNumberAreaWidth();
    void    highlightCurrentLine();
    void    updateLineNumberArea(const QRect&, int);

private:
    QWidget *lineNumberArea_;
};

#endif // TEXTEDIT_H
