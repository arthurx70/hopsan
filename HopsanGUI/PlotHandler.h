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
//! @file   PlotHandler.h
//! @author Peter Nordin <robert.braun@liu.se>
//! @date   2013
//!
//! @brief Contains a handler for plotting
//!
//$Id$

#ifndef PLOTHANDLER_H
#define PLOTHANDLER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QColor>
#include <QSharedPointer>

// Forward Declarations
class ContainerObject;
class PlotWindow;
class VectorVariable;

#include "LogVariable.h"

class PlotHandler : public QObject
{
    Q_OBJECT
public:
    explicit PlotHandler(QObject *pParent=0);
    ~PlotHandler();

    void createNewPlotWindowIfItNotAlreadyExists(QString name="");
    PlotWindow *createNewUniquePlotWindow(const QString &rName);
    PlotWindow *createNewPlotWindowOrGetCurrentOne(QString name="");
    PlotWindow *createNewOrReplacePlotwindow(const QString &rName="");
    PlotWindow *getPlotWindow(const QString &rName);

    PlotWindow *plotDataToWindow(PlotWindow *pPlotWindow,   SharedVectorVariableT data,  int axis, bool autoRefresh=true, QColor curveColor=QColor(), int type=1, int thickness=2);
    PlotWindow *plotDataToWindow(PlotWindow *pPlotWindow,   SharedVectorVariableT xdata, SharedVectorVariableT ydata, int axis, bool autoRefresh=true, QColor curveColor=QColor(), int type=1, int thickness=2);
    PlotWindow *setPlotWindowXData(PlotWindow *pPlotWindow, SharedVectorVariableT xdata, bool force=false);

    PlotWindow *plotDataToWindow(QString windowName,        SharedVectorVariableT data,  int axis, QColor curveColor=QColor(), int type=1, int thickness=2);
    PlotWindow *plotDataToWindow(QString windowName,        SharedVectorVariableT xdata, SharedVectorVariableT ydata, int axis, QColor curveColor=QColor(), int type=1, int thickness=2);


    void closeWindow(const QString &rWindowName);
    void closeAllOpenWindows();

signals:
    
protected slots:
    void forgetPlotWindow(PlotWindow *pWindow);

private:
    QMap<QString, PlotWindow*> mOpenPlotWindows;
    
};

#endif // PLOTHANDLER_H
