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
//! @file   GUIPortAppearance.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the GUIPortAppearance class
//!
//$Id$

#include "GUIPortAppearance.h"
#include "common.h"
#include "CoreAccess.h"

#include <QDebug>

PortAppearance::PortAppearance()
{
    //Default values
    mEnabled = true;
    mAutoPlaced = true;
    mPoseModified = false;
}

//! @brief Contains hardcoded appearance for different hopsancore ports
//! @todo maybe this should be placed in some more generic external .txt file in some way
void PortAppearance::selectPortIcon(QString CQSType, QString porttype, QString nodetype)
{
    mMainIconPath.clear();

    mMainIconPath = QString(PORTICONPATH);
    if (nodetype == "NodeSignal")
    {
        mMainIconPath.append("SignalPort");
        if ( porttype == "ReadPortType" || porttype == "ReadMultiportType")
        {
            mMainIconPath.append("_read");
        }
        else
        {
            mMainIconPath.append("_write");
        }
        mCQSOverlayPath.clear();
    }
    else
    {
        QString niceName = NodeInfo(nodetype).niceName;
        if(niceName.isEmpty())
        {
            mMainIconPath.append("SystemPort");
        }
        else
        {
            niceName[0] = niceName[0].toUpper();
            mMainIconPath.append(niceName+"Port");
        }

        //Select cqs overlay icon path depending on cqs type
        if (CQSType == "C")
        {
            mCQSOverlayPath = (QString(PORTICONPATH) + "PortOverlayC.svg");
        }
        else if (CQSType == "Q")
        {
            mCQSOverlayPath = (QString(PORTICONPATH) + "PortOverlayQ.svg");
        }
        else if (CQSType == "UndefinedCQSType")
        {
            mCQSOverlayPath = (QString(PORTICONPATH) + "PortOverlayUnknown.svg");
        }
        else
        {
            mCQSOverlayPath.clear();
        }
    }
    mMainIconPath.append(".svg");

    //qDebug() << "mMainIconPath: " << mMainIconPath;

    //Check if we need to add multiport overlay
    if (porttype.contains("MultiportType"))
    {
        mMultiPortOverlayPath = (QString(PORTICONPATH) + "MultiPortOverlay.svg");
    }
    else
    {
        mMultiPortOverlayPath.clear();
    }
}

