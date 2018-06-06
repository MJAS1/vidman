/****************************************************************************
**
** highlighter.cpp
**
** Original file:
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** Modifications:
** Author: Manu Sutela
** Copyright (C) 2018 Aalto University
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    typeFormat.setForeground(Qt::darkYellow);
    QStringList typePatterns;
    typePatterns << "\\bfade_in\\b" << "\\bfade_out\\b" << "\\bflip\\b"
                 << "\\bimage\\b" << "\\btext\\b" << "\\bfreeze\\b"
                 << "\\brotate\\b" << "\\brts\\b" << "\\bdtr\\b"
                 << "\\bdetect_motion\\b" << "\\brecord\\b" << "\\bplayback\\b"
                 << "\\bvideo\\b";

    foreach(const QString& pattern, typePatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = typeFormat;
        highlightingRules.append(rule);
    }

    keywordFormat.setForeground(Qt::darkRed);
    QStringList keywordPatterns;
    keywordPatterns << "\\btype\\b" << "\\bstart\\b" << "\\bduration\\b" << "\\bx\\b"
                    << "\\by\\b"<< "\\bfilename\\b" << "\\bid\\b" << "\\bobject_id\\b"
                    << "\\bangle\\b" << "\\bdelay\\b" << "\\bcolor\\b"
                    << "\\btrig_code\\b" << "\\bstring\\b" << "\\btarget\\b"
                    << "\\btolerance\\b" << "\\baxis\\b" << "\\bsuccess_code\\b"
                    << "\\bfail_code\\b";

    foreach(const QString& pattern, keywordPatterns)
    {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    blockFormat.setForeground(Qt::darkRed);
    blockFormat.setFontWeight(QFont::Bold);
    QStringList blockPatterns;
    blockPatterns << "event" << "delete" << "\\bobject\\b" << "pause";

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
    commentStartExpression = QRegExp("^#");
    commentEndExpression = QRegExp("$");
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
