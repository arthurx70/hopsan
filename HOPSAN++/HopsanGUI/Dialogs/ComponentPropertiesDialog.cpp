//!
//! @file   ComponentPropertiesDialog.cpp
//! @author Björn Eriksson <bjorn.eriksson@liu.se>
//! @date   2010-03-01
//!
//! @brief Contains a dialog class for changing component properties
//!
//$Id$

#include <QtGui>
#include <cassert>
#include <iostream>

#include "ComponentPropertiesDialog.h"
#include "../MainWindow.h"
#include "../GUIPort.h"
#include "../Widgets/MessageWidget.h"
#include "../GUIObjects/GUIComponent.h"
#include "../GUIObjects/GUIContainerObject.h"
#include "../GUIObjects/GUISystem.h"
#include "../UndoStack.h"
#include "../Widgets/ProjectTabWidget.h"
#include "../Widgets/SystemParametersWidget.h"


//! @class ComponentPropertiesDialog
//! @brief The ComponentPropertiesDialog class is a Widget used to interact with component parameters.
//!
//! It reads and writes parameters to the core components.
//!


//! @brief Constructor for the parameter dialog for components
//! @param pGUIComponent Pointer to the component
//! @param parent Pointer to the parent widget
ComponentPropertiesDialog::ComponentPropertiesDialog(GUIComponent *pGUIComponent, QWidget *parent)
    : QDialog(parent)
{
    mpGUIComponent = pGUIComponent;
    createEditStuff();
}


//! @brief Creates the contents in the parameter dialog
void ComponentPropertiesDialog::createEditStuff()
{
    mpNameEdit = new QLineEdit(mpGUIComponent->getName(), this);

    QFont fontH1;
    fontH1.setBold(true);

    QFont fontH2;
    fontH2.setBold(true);
    fontH2.setItalic(true);

    QLabel *pParameterLabel = new QLabel("Parameters", this);
    pParameterLabel->setFont(fontH1);

    QGridLayout *parameterLayout = new QGridLayout();

    QVector<QString> parnames = mpGUIComponent->getParameterNames();
    QVector<QString>::iterator pit;
    size_t n = 0;
    for ( pit=parnames.begin(); pit!=parnames.end(); ++pit )
    {
        QString valueTxt = mpGUIComponent->getParameterValueTxt(*pit);
        bool ok;
        valueTxt.toDouble(&ok);
        if((!ok) && !(mpGUIComponent->mpParentContainerObject->getCoreSystemAccessPtr()->hasSystemParameter(valueTxt)))
        {
            gpMainWindow->mpMessageWidget->printGUIWarningMessage(tr("Global parameter no longer exists, replacing with last used value."));
        }

        mvParameterLayout.push_back(new ParameterLayout(*pit,
                                                        mpGUIComponent->getParameterDescription(*pit),
                                                        valueTxt,
                                                        mpGUIComponent->getParameterUnit(*pit),
                                                        mpGUIComponent));

        parameterLayout->addLayout(mvParameterLayout.back(), n, 0);
        ++n;
    }


    QGridLayout *startValueLayout = new QGridLayout();
    size_t sr=0;
    QLabel *pStartValueLabel = new QLabel("Start Values", this);
    pStartValueLabel->setFont(fontH1);

    QList<GUIPort*> ports = mpGUIComponent->getPortListPtrs();
    QList<GUIPort*>::iterator portIt;
    double j=0;
    QVector<QVector<QString> > startDataNamesStr, startDataUnitsStr;
    QVector<QVector<double> > startDataValuesDbl;
    QVector<QVector<QString> > startDataValuesTxt;
    startDataNamesStr.resize(ports.size());
    startDataValuesDbl.resize(ports.size());
    startDataValuesTxt.resize(ports.size());
    startDataUnitsStr.resize(ports.size());
    mvStartValueLayout.resize(ports.size());
    for ( portIt=ports.begin(); portIt!=ports.end(); ++portIt )
    {
        (*portIt)->getStartValueDataNamesValuesAndUnits(startDataNamesStr[j], startDataValuesTxt[j], startDataUnitsStr[j]);
        if(!(startDataNamesStr[j].isEmpty()))
        {
            QString portName("Port, ");
            portName.append((*portIt)->getName());
            QLabel *portLabelName = new QLabel(portName, this);
            portLabelName->setFont(fontH2);
            startValueLayout->addWidget(portLabelName, sr, 0);
            ++sr;

            mvStartValueLayout[j].resize(startDataNamesStr[j].size());
            for(int i=0; i < startDataNamesStr[j].size(); ++i)
            {
                mvStartValueLayout[j][i]= new ParameterLayout("",
                                                              startDataNamesStr[j][i],
                                                              startDataValuesTxt[j][i],
                                                              startDataUnitsStr[j][i],
                                                              mpGUIComponent);
                startValueLayout->addLayout(mvStartValueLayout[j][i], sr, 0);

                ++sr;
            }
            ++j;
        }
    }


    //qDebug() << "after parnames";

    okButton = new QPushButton(tr("&Ok"), this);
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("&Cancel"), this);
    cancelButton->setDefault(false);

    buttonBox = new QDialogButtonBox(Qt::Vertical, this);
    buttonBox->addButton(okButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(cancelButton, QDialogButtonBox::ActionRole);

    connect(okButton, SIGNAL(pressed()), SLOT(okPressed()));
    connect(cancelButton, SIGNAL(pressed()), SLOT(close()));

    QHBoxLayout *pNameLayout = new QHBoxLayout();
    QLabel *pNameLabel = new QLabel("Name: ", this);
    pNameLayout->addWidget(pNameLabel);
    pNameLayout->addWidget(mpNameEdit);

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    int lr = 0; //Layout row
    mainLayout->addLayout(pNameLayout, lr, 0);
    mainLayout->addWidget(buttonBox, lr, 1);
    ++lr;

    if(!(mvParameterLayout.empty()))
    {
        mainLayout->addWidget(pParameterLabel, lr, 0);
        ++lr;
        mainLayout->addLayout(parameterLayout, lr, 0);
        ++lr;
    }
    if(!(mvStartValueLayout[0].isEmpty()))
    {
        mainLayout->addWidget(pStartValueLabel,lr, 0);
        ++lr;
        mainLayout->addLayout(startValueLayout, lr, 0);
    }
    setLayout(mainLayout);

    setWindowTitle(tr("Parameters"));
}


//! @brief Reads the values from the dialog and writes them into the core component
void ComponentPropertiesDialog::okPressed()
{
    mpGUIComponent->mpParentContainerObject->renameGUIModelObject(mpGUIComponent->getName(), mpNameEdit->text());
    //qDebug() << mpNameEdit->text();

    setParametersAndStartValues();
}


//! @brief Sets the parameters and start values in the core component. Read the values from the dialog and write them into the core component.
void ComponentPropertiesDialog::setParametersAndStartValues()
{
    //! @todo Maybe only use strings as value to parameters and start values and interpret it in core, this opens up for aritmetric expressions as well

    //Parameters
    bool addedUndoPost = false;
    for (int i=0 ; i < mvParameterLayout.size(); ++i )
    {
        QString valueTxt = mvParameterLayout[i]->getDataValueTxt();

        //Get the old value to se if it changed
        QString oldValueTxt = mpGUIComponent->getParameterValueTxt(mvParameterLayout[i]->getDataName());
        //Parameter has changed, add to undo stack
        if(oldValueTxt != valueTxt)
        {
            if(!addedUndoPost)
            {
                this->mpGUIComponent->mpParentContainerObject->mUndoStack->newPost("changedparameters");
                addedUndoPost = true;
            }
            this->mpGUIComponent->mpParentContainerObject->mUndoStack->registerChangedParameter(mpGUIComponent->getName(),
                                                                                                mvParameterLayout[i]->getDataName(),
                                                                                                oldValueTxt,
                                                                                                valueTxt);
            mpGUIComponent->mpParentContainerObject->mpParentProjectTab->hasChanged();
        }
        //Set the parameter
        if(!mpGUIComponent->setParameterValue(mvParameterLayout[i]->getDataName(), valueTxt))
        {
            QMessageBox::critical(0, "Hopsan GUI",
                                  QString("'%1' is an invalid value for parameter '%2'.")
                                  .arg(valueTxt)
                                  .arg(mvParameterLayout[i]->getDataName()));
            mvParameterLayout[i]->setDataValueTxt(oldValueTxt);
            return;
        }
    }

    //StartValues
    QList<GUIPort*> ports = mpGUIComponent->getPortListPtrs();
    QList<GUIPort*>::iterator portIt;
    int j = 0;
    for(portIt=ports.begin(); portIt!=ports.end(); ++portIt)
    {
        for(int i=0; i < mvStartValueLayout[j].size(); ++i)
        {
            QString valueTxt = mvStartValueLayout[j][i]->getDataValueTxt();

            //Get the old value to se if it changed
            QString oldValueTxt = mpGUIComponent->getStartValueTxt((*portIt)->getName(), mvStartValueLayout[j][i]->getDescriptionName());
            //Parameter has changed, add to undo stack
            if(oldValueTxt != valueTxt)
            {
                if(!addedUndoPost)
                {
                    this->mpGUIComponent->mpParentContainerObject->mUndoStack->newPost("changedparameters");
                    addedUndoPost = true;
                }
                this->mpGUIComponent->mpParentContainerObject->mUndoStack->registerChangedStartValue(mpGUIComponent->getName(),
                                                                                                     (*portIt)->getName(),
                                                                                                     mvStartValueLayout[j][i]->getDescriptionName(),
                                                                                                     oldValueTxt,
                                                                                                     valueTxt);
                mpGUIComponent->mpParentContainerObject->mpParentProjectTab->hasChanged();
            }
            //Set the start value
            if(!mpGUIComponent->setStartValue((*portIt)->getName(), mvStartValueLayout[j][i]->getDescriptionName(), valueTxt))
            {
                QMessageBox::critical(0, "Hopsan GUI",
                                      QString("'%1' is an invalid value for start value '%2'.")
                                      .arg(valueTxt)
                                      .arg(mvStartValueLayout[j][i]->getDescriptionName()));
                mvStartValueLayout[j][i]->setDataValueTxt(oldValueTxt);
                return;
            }
        }
        ++j;
    }

    std::cout << "Parameters and start values updated." << std::endl;
    this->close();
}






ParameterLayout::ParameterLayout(QString dataName, QString descriptionName, double dataValue, QString unitName, GUIModelObject *pGUIModelObject, QWidget *parent)
    : QGridLayout(parent)
{
    QString dataValueStr;
    dataValueStr.setNum(dataValue);
    commonConstructorCode(dataName, descriptionName, dataValueStr, unitName, pGUIModelObject);
}


ParameterLayout::ParameterLayout(QString dataName, QString descriptionName, QString dataValue, QString unitName, GUIModelObject *pGUIModelObject, QWidget *parent)
    : QGridLayout(parent)
{
    commonConstructorCode(dataName, descriptionName, dataValue, unitName, pGUIModelObject);
}


void ParameterLayout::commonConstructorCode(QString dataName, QString descriptionName, QString dataValue, QString unitName, GUIModelObject *pGUIModelObject)
{
    mpGUIModelObject = pGUIModelObject;

    mDescriptionNameLabel.setMinimumWidth(100);
    mDescriptionNameLabel.setMaximumWidth(100);
    mDataNameLabel.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    mDataNameLabel.setMinimumWidth(50);
    mDataNameLabel.setMaximumWidth(50);
    mDataValuesLineEdit.setMinimumWidth(100);
    mDataValuesLineEdit.setMaximumWidth(100);
    mUnitNameLabel.setMinimumWidth(50);
    mUnitNameLabel.setMaximumWidth(50);

    mSystemParameterToolButton.setIcon(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"));

    mDataNameLabel.setText(dataName);
    mDescriptionNameLabel.setText(descriptionName);
    mDataValuesLineEdit.setText(dataValue);
    mUnitNameLabel.setText(unitName);

    addWidget(&mDescriptionNameLabel, 0, 0);
    addWidget(&mDataNameLabel, 0, 1);
    addWidget(&mDataValuesLineEdit, 0, 2);
    addWidget(&mSystemParameterToolButton, 0, 3);
    addWidget(&mUnitNameLabel, 0, 4);

    connect(&mSystemParameterToolButton, SIGNAL(pressed()), this, SLOT(showListOfSystemParameters()));
}


QString ParameterLayout::getDescriptionName()
{
    return mDescriptionNameLabel.text();
}


QString ParameterLayout::getDataName()
{
    return mDataNameLabel.text();
}


double ParameterLayout::getDataValue()
{
    return mDataValuesLineEdit.text().toDouble();
}

QString ParameterLayout::getDataValueTxt()
{
    return mDataValuesLineEdit.text();
}


void ParameterLayout::setDataValueTxt(QString valueTxt)
{
    mDataValuesLineEdit.setText(valueTxt);
}


void ParameterLayout::showListOfSystemParameters()
{
    //mSystemParameterToolButton.setDown(false);

    QMenu menu;

    QMap<std::string, double> SystemMap = gpMainWindow->mpProjectTabs->getCurrentSystem()->getCoreSystemAccessPtr()->getSystemParametersMap();
    QMap<std::string, double>::iterator it;
    for(it=SystemMap.begin(); it!=SystemMap.end(); ++it)
    {
        QString valueString;
        valueString.setNum(it.value());
        QAction *tempAction = menu.addAction(QString(it.key().c_str()));
        tempAction->setIconVisibleInMenu(false);
    }

    QCursor cursor;
    QAction *selectedAction = menu.exec(cursor.pos());
    if(selectedAction != 0)
    {
        mDataValuesLineEdit.setText(selectedAction->text());
    }
}
