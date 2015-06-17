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
//! @file   HelpDialog.cpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2010-XX-XX
//!
//! @brief Contains a class for the Help dialog
//!
//$Id$

//Qt includes
#include <QWebView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>

//Hopsan includes
#include "common.h"
#include "global.h"
#include "DesktopHandler.h"
#include "HelpDialog.h"

//! @class HelpDialog
//! @brief A class for displaying the "Help" dialog
//!
//! Shows the HTML user guide from Doxygen
//!

//! Constructor for the help dialog
//! @param parent Pointer to the main window
HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowIcon(QIcon(QString(QString(ICONPATH) + "hopsan.png")));
    this->setObjectName("HelpDialog");
    this->setWindowTitle("Hopsan User Guide");
    this->setMinimumSize(640, 480);
    this->setWindowModality(Qt::NonModal);

    mpHelp = new QWebView(this);

    QAction *pBackAction = mpHelp->pageAction(QWebPage::Back);
    pBackAction->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-StepLeft.png")));
    QAction *pForwardAction = mpHelp->pageAction(QWebPage::Forward);
    pForwardAction->setIcon(QIcon(QString(QString(ICONPATH) + "Hopsan-StepRight.png")));

    QToolBar *pToolBar = new QToolBar(this);
    pToolBar->addAction(pBackAction);
    pToolBar->addAction(pForwardAction);

    QVBoxLayout *pLayout = new QVBoxLayout(this);
    pLayout->addWidget(pToolBar);
    pLayout->addWidget(mpHelp);
    pLayout->setStretch(1,1);
    this->setLayout(pLayout);

    //! @todo Set size depending one screen size
    this->resize(1024,768);
}


void HelpDialog::open()
{
    mpHelp->load(QUrl::fromLocalFile(gpDesktopHandler->getHelpPath() + "index.html"));

    //Using show instead of open for modaless window
    QDialog::show();
}


void HelpDialog::open(QString file)
{
    mpHelp->load(QUrl::fromLocalFile(gpDesktopHandler->getHelpPath() + file));

    //Using show instead of open for modaless window
    QDialog::show();
}


void HelpDialog::centerOnScreen()
{
    int sx = qApp->desktop()->screenGeometry().center().x();
    int sy = qApp->desktop()->screenGeometry().center().y();
    int w = this->width();
    int h = this->height();

    this->move(sx-w/2, sy-h/2);
}

