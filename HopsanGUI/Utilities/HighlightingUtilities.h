/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//!
//! @file   HighlightingUtilities.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2013-08-23
//!
//! @brief Contains highlighter classes for various languages
//!
//$Id$

#ifndef HIGHLIGHTINGUTILITIES_H
#define HIGHLIGHTINGUTILITIES_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QColor>

class XmlHighlighter : public QSyntaxHighlighter
{
public:
	XmlHighlighter(QTextDocument* parent);

    enum HighlightType
	{
		SyntaxChar,
        ElementName,
		Comment,
		AttributeName,
		AttributeValue,
		Error,
		Other
	};

	void setHighlightColor(HighlightType type, QColor color, bool foreground = true);
	void setHighlightFormat(HighlightType type, QTextCharFormat format);

protected:
	void highlightBlock(const QString& rstrText);
	int  processDefaultText(int i, const QString& rstrText);

private:
    QTextCharFormat mSyntaxChar;
    QTextCharFormat mElementName;
    QTextCharFormat mComment;
    QTextCharFormat mAttributeName;
    QTextCharFormat mAttributeValue;
    QTextCharFormat mError;
    QTextCharFormat mOther;

    enum ParsingState
    {
        NoState = 0,
        ExpectElementNameOrSlash,
        ExpectElementName,
        ExpectAttributeOrEndOfElement,
        ExpectEqual,
        ExpectAttributeValue
    };

    enum BlockState
    {
        NoBlock = -1,
        InComment,
        InElement
    };

    ParsingState state;
};


class CppHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CppHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat preProcessorFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat tagFormat;
    QTextCharFormat functionFormat;
};



class ModelicaHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    ModelicaHighlighter(QTextDocument *parent = 0);

protected:
    QStringList getSupportedFunctionsList();
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
};


class PythonHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    PythonHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat tagFormat;
    QTextCharFormat functionFormat;
};


class HcomHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    HcomHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat commandFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat tagFormat;
    QTextCharFormat functionFormat;
};

#endif // HIGHLIGHTINGUTILITIES_H
