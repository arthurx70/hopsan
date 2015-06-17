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
//! @file   EditComponentDialog.h
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-12-01
//!
//! @brief Contains a class for the component generator dialog
//!
//$Id$

#ifndef EDITCOMPONENTDIALOG_H_INCLUDED
#define EDITCOMPONENTDIALOG_H_INCLUDED

#include <QDialog>
#include <QWizard>

class QTextDocument;
class QVBoxLayout;
class QComboBox;
class QTextEdit;
class QDialogButtonBox;
class QToolButton;
class QWizardPage;
class QLabel;
class QLineEdit;
class QGridLayout;
class QDoubleSpinBox;
class QSpinBox;

class ModelObjectAppearance;

class EditComponentDialog : public QDialog
{
    Q_OBJECT
public:
    enum SourceCodeEnumT {Cpp, Modelica};
    QVBoxLayout *mpVerticalLayout;
    QTextEdit *mpCodeTextEdit;
    QComboBox *mpSolverComboBox;
    QDialogButtonBox *mpButtonBox;
    QToolButton *mpLoadButton;
    QToolButton *mpSaveButton;

    EditComponentDialog(QString code, SourceCodeEnumT language, QWidget *parent=0);

    void retranslateUi();

    QString getCode();
    int getSolver();

    void openCreateComponentWizard(SourceCodeEnumT language);

    void doLoad(QString path);
    void doSave(QString path);

    QString mLastLoadedOrSavedCode;

private slots:
    //void openCreateComponentWizard(SourceCodeEnumT language);
    void setHighlighter(SourceCodeEnumT language);
    void load();
    void save();
};


class CreateComponentWizard : public QWizard
{
    Q_OBJECT

public:
    CreateComponentWizard(EditComponentDialog::SourceCodeEnumT language, EditComponentDialog *parent = 0);

private slots:
    void updatePage(int i);
    void generate();

private:
    EditComponentDialog::SourceCodeEnumT mLanguage;

    EditComponentDialog *mpParent;

    QWizardPage *mpFirstPage;
    QGridLayout *mpFirstPageLayout;
    QLabel *mpTypeNameLabel;
    QLineEdit *mpTypeNameLineEdit;
    QLabel *mpDisplayNameLabel;
    QLineEdit *mpDisplayNameLineEdit;
    QLabel *mpCqsTypeLabel;
    QComboBox *mpCqsTypeComboBox;
    QLabel *mpNumberOfPortsLabel;
    QSpinBox *mpNumberOfPortsSpinBox;
    QLabel *mpNumberOfParametersLabel;
    QSpinBox *mpNumberOfParametersSpinBox;

    QWizardPage *mpSecondPage;
    QGridLayout *mpSecondPageLayout;
    QLabel *mpPortIdTitle;
    QLabel *mpPortNameTitle;
    QLabel *mpPortTypeTitle;
    QLabel *mpNodeTypeTitle;
    QLabel *mpDefaultValueTitle;
    QList<QLabel*> mPortIdPtrs;
    QList<QLineEdit*> mPortNameLineEditPtrs;
    QList<QComboBox*> mPortTypeComboBoxPtrs;
    QList<QDoubleSpinBox*> mPortDefaultSpinBoxPtrs;

    QWizardPage *mpThirdPage;
    QGridLayout *mpThirdPageLayout;
    QLabel *mpParameterNameTitle;
    QLabel *mpParameterUnitTitle;
    QLabel *mpParameterDescriptionTitle;
    QLabel *mpParameterValueTitle;
    QList<QLineEdit*> mParameterNameLineEditPtrs;
    QList<QLineEdit*> mParameterUnitLineEditPtrs;
    QList<QLineEdit*> mParameterDescriptionLineEditPtrs;
    QList<QLineEdit*> mParameterValueLineEditPtrs;

};

#endif // EDITCOMPONENTDIALOG_H_INCLUDED
