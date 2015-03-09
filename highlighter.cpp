#include "highlighter.h"
#include <iostream>

Highlighter::Highlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    typeFormat.setForeground(Qt::darkYellow);
    QStringList typePatterns;
    typePatterns << "\\bfade in\\b" << "\\bfadein\\b" << "\\bfadeout\\b"
                    << "\\bfade out\\b" << "\\bflip\\b" << "\\bimage\\b"
                    << "\\btext\\b" << "\\bfreeze\\b" << "\\brotate\\b"
                    << "\\brts\\b" << "\\bdtr\\b" << "\\bdetectmotion\\b"
                    << "\\brecord\\b" << "\\bplayback\\b";

    foreach(const QString& pattern, typePatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = typeFormat;
        highlightingRules.append(rule);
    }

    keywordFormat.setForeground(Qt::darkRed);
    QStringList keywordPatterns;
    keywordPatterns << "\\btype\\b" << "\\bstart\\b" << "\\bduration\\b" << "\\bx\\b"
                    << "\\by\\b"<< "\\bfilename\\b" << "\\bid\\b" << "\\bimageid\\b"
                    << "\\bimage id\\b" << "\\bangle\\b" << "\\bdelay\\b" << "\\bcolor\\b"
                    << "\\btrigcode\\b" << "\\blength\\b" << "\\bvideoid\\b";

    foreach(const QString& pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    blockFormat.setForeground(Qt::darkRed);
    blockFormat.setFontWeight(QFont::Bold);
    QStringList blockPatterns;
    blockPatterns << "event" << "removeevent" << "remove event"
                  << "imageobject" << "image object" << "videoobject";

    foreach(const QString& pattern, blockPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = blockFormat;
        highlightingRules.append(rule);
    }

    numFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegExp("\\d");
    rule.format = numFormat;
    highlightingRules.append(rule);

    commentFormat.setForeground(Qt::darkGreen);
    commentStartExpression = QRegExp("^#comment", Qt::CaseInsensitive);
    commentEndExpression = QRegExp("#endcomment|#end comment", Qt::CaseInsensitive);

}

void Highlighter::highlightBlock(const QString &text)
{
    foreach(const HighlightingRule& rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text.toLower());
        while(index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text.toLower(), index + length);

        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if(previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text.simplified());

    while(startIndex >= 0)
    {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if(endIndex==-1)
        {
            setCurrentBlockState(1);
            commentLength = text.length()-startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }

}
