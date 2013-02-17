/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   ComponentGeneratorUtilities.h
//! @author Robert Braun <robert.braun@liu.se
//! @date   2012-01-08
//!
//! @brief Contains component generation utiluties
//!
//$Id$


#ifndef COMPONENTGENERATORUTILITIES_H
#define COMPONENTGENERATORUTILITIES_H

#include <QPointF>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QDomElement>
#include "win32dll.h"

#include "symhop/SymHop.h"
#include "GeneratorUtilities.h"

//class ModelObjectAppearance;

namespace hopsan {
class ComponentSystem;
}


class DLLIMPORTEXPORT HopsanGenerator
{
public:
    HopsanGenerator(QString coreIncludePath, QString binPath, bool showDialog=false);
    void printMessage(QString msg);
    void printErrorMessage(QString msg);
    QString generateSourceCodefromComponentObject(ComponentSpecification comp, bool overwriteStartValues=false);
    void compileFromComponentObject(QString outputFile, ComponentSpecification comp, bool overwriteStartValues=false);

    QString getCoreIncludePath();
    QString getBinPath();

    bool assertFilesExist(QString path, QStringList files);

    void callProcess(QString name, QStringList args, QString workingDirectory=QString());
    bool runUnixCommand(QString cmd);

protected:
    QString mOutputPath;
    QString mTempPath;
    QString mCoreIncludePath;
    QString mBinPath;

    QTextEdit *mpTextEdit;
    QVBoxLayout *mpLayout;
    QPushButton *mpDoneButton;
    QWidget *mpDialog;

    bool mShowDialog;
};


#endif // COMPONENTGENERATORUTILITIES_H
