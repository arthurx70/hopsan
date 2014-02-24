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
//! @file   MainWindow.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the HopsanGUI MainWindow class
//!
//$Id$

#define MINSTARTTIME -1000000
#define MAXSTARTTIME 1000000
#define MINTIMESTEP 0.0
#define MAXTIMESTEP 1000000
#define MINSTOPTIME -1000000
#define MAXSTOPTIME 1000000

#include <QDebug>
#include <QFontDatabase>
#include <QtNetwork>
#include <QPixmap>
#include <QDesktopWidget>
#include <QDockWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QDesktopServices>


#include "common.h"
#include "global.h"
#include "Configuration.h"
#include "CopyStack.h"
#include "DesktopHandler.h"
#include "LibraryHandler.h"
#include "MainWindow.h"
#include "ModelHandler.h"
#include "PlotHandler.h"
#include "UndoStack.h"
#include "version_gui.h"
#include "HcomHandler.h"

#include "Widgets/DebuggerWidget.h"
#include "Widgets/PlotWidget.h"
#include "Widgets/MessageWidget.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/LibraryWidget.h"
#include "Widgets/ModelWidget.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/PyDockWidget.h"
#include "Widgets/SystemParametersWidget.h"
#include "Widgets/UndoWidget.h"
#include "Widgets/WelcomeWidget.h"
#include "Widgets/HVCWidget.h"
#include "Widgets/DataExplorer.h"

#include "Dialogs/OptionsDialog.h"
#include "Dialogs/AboutDialog.h"
#include "Dialogs/HelpDialog.h"
#include "Dialogs/OptimizationDialog.h"
#include "Dialogs/SensitivityAnalysisDialog.h"

#include "Utilities/GUIUtilities.h"
#include "Utilities/HelpPopUpWidget.h"

//! @todo maybe we can make sure that we dont need to include these here
#include "GraphicsView.h"
#include "GUIObjects/GUISystem.h"

// Declare (create) global pointers that will point to MainWindow children
PlotHandler *gpPlotHandler = 0;
LibraryWidget *gpLibraryWidget = 0;
TerminalWidget *gpTerminalWidget = 0;
ModelHandler *gpModelHandler = 0;
PlotWidget *gpPlotWidget = 0;
CentralTabWidget *gpCentralTabWidget = 0;
SystemParametersWidget *gpSystemParametersWidget = 0;
UndoWidget *gpUndoWidget = 0;
LibraryHandler *gpLibraryHandler = 0;

//! @brief Constructor for main window
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->hide();

    // Set main window options
    this->setDockOptions(QMainWindow::ForceTabbedDocks);
    this->setMouseTracking(true);

    // Set name and icon of main window
#ifdef DEVELOPMENT
    this->setWindowTitle(tr("Hopsan (development version)"));
#else
    this->setWindowTitle(tr("Hopsan"));
#endif
    this->setWindowIcon(QIcon(QString(QString(ICONPATH) + tr("hopsan.png"))));

    // Set dock widget corner owner
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    // Set the size and position of the main window
    int sh = qApp->desktop()->screenGeometry().height();
    int sw = qApp->desktop()->screenGeometry().width();
    this->resize(sw*0.8, sh*0.8);   //Resize window to 80% of screen height and width
//    int w = this->size().width();
//    int h = this->size().height();
//    int x = (sw - w)/2;
//    int y = (sh - h)/2;
//    this->move(x, y);       //Move window to center of screen
}


//! @brief Destructor
//! @todo Shouldn't all member pointers be deleted here?
MainWindow::~MainWindow()
{
    delete gpPlotHandler;
    delete mpCentralTabs;
    delete mpModelHandler;
    delete mpMenuBar;
    delete mpStatusBar;
}

//! @brief Creates the contents of the MainWindow, call this only once
void MainWindow::createContents()
{
    // Create plothandler as child to mainwindo but assign to global ptr
    gpPlotHandler = new PlotHandler(this);

    //Create the terminal widget
    mpTerminalDock = new QDockWidget(tr("Terminal"), this);
    mpTerminalDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    gpTerminalWidget = new TerminalWidget(this);
    gpTerminalWidget->mpConsole->printFirstInfo();
    mpTerminalDock->setWidget(gpTerminalWidget);
    mpTerminalDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpTerminalDock);
    mpTerminalDock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(gpMessageHandler, SIGNAL(newAnyMessage(GUIMessage)), gpTerminalWidget, SLOT(printMessage(GUIMessage)));

    //Create the message widget and its dock (must be done before everything that uses it!)
    mpMessageDock = new QDockWidget(tr("Messages"), this);
    mpMessageDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    mpMessageWidget = new MessageWidget(this);
    mpMessageDock->setWidget(mpMessageWidget);
    mpMessageDock->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpMessageDock);
    connect(gpMessageHandler, SIGNAL(newAnyMessage(GUIMessage)), mpMessageWidget, SLOT(receiveMessage(GUIMessage)));

    gpMessageHandler->collectHopsanCoreMessages();
#ifdef HOPSANCOMPILED64BIT
    gpMessageHandler->addInfoMessage("HopsanGUI 64-bit, Version: " + QString(HOPSANGUIVERSION));
#else
    gpMessageHandler->addInfoMessage("HopsanGUI 32-bit, Version: " + QString(HOPSANGUIVERSION));
#endif

    //Load configuration from settings file
    gpSplash->showMessage("Loading configuration...");
    gpConfig->loadFromXml();      //!< @todo This does not really belong in main window constructor, but it depends on main window so keep it for now

    gpSplash->showMessage("Initializing GUI...");

    //Create dialogs
    mpAboutDialog = new AboutDialog(this);
    mpOptimizationDialog = new OptimizationDialog(this);
    mpHelpDialog = new HelpDialog(0);

    //Create the Python widget
#ifdef USEPYTHONQT
    mpPyDockWidget = new PyDockWidget(this);
    mpPyDockWidget->setFeatures(QDockWidget::DockWidgetVerticalTitleBar | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::BottomDockWidgetArea, mpPyDockWidget);
#else
    mpPyDockWidget = 0;
#endif

    //Create the library handler
    gpLibraryHandler = new LibraryHandler();

    //Create the component library widget and its dock
    mpLibDock = new QDockWidget(tr("Component Library"), this);
    mpLibDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpLibraryWidget = new LibraryWidget(this);
    gpLibraryWidget = mpLibraryWidget;
    mpLibDock->setWidget(mpLibraryWidget);
    addDockWidget(Qt::LeftDockWidgetArea, mpLibDock);

    //Create the statusbar widget
    mpStatusBar = new QStatusBar();
    mpStatusBar->setObjectName("statusBar");
    this->setStatusBar(mpStatusBar);

    //Create the undo widget and the options dialog
    mpUndoWidget = new UndoWidget(this);
    gpUndoWidget = mpUndoWidget;
    mpOptionsDialog = new OptionsDialog(this);

    //Create the central widget for the main window
    mpCentralWidget = new QWidget(this);
    mpCentralWidget->setObjectName("centralwidget");
    mpCentralWidget->setMouseTracking(true);
    this->setCentralWidget(mpCentralWidget);

    //Create the grid layout for the centralwidget
    mpCentralGridLayout = new QGridLayout(mpCentralWidget);
    mpCentralGridLayout->setContentsMargins(4,4,4,4);

    //Create the model handler object
    mpModelHandler = new ModelHandler(this);
    gpModelHandler = mpModelHandler;
    connect(mpModelHandler, SIGNAL(modelChanged(ModelWidget*)), gpTerminalWidget->mpHandler, SLOT(setModelPtr(ModelWidget*)));

    //Create the sensitivity analysis dialog
    mpSensitivityAnalysisDialog = new SensitivityAnalysisDialog(this);

    //Create the main tab container, need at least one tab
    mpCentralTabs = new CentralTabWidget(this);
    gpCentralTabWidget = mpCentralTabs;
    mpCentralTabs->setObjectName("centralTabs");
    mpCentralTabs->setMouseTracking(true);
    mpCentralGridLayout->addWidget(mpCentralTabs,0,0,4,4);

    //Create the system parameter widget and hide it
    mpSystemParametersWidget = new SystemParametersWidget(this);
    gpSystemParametersWidget = mpSystemParametersWidget;
    mpSystemParametersWidget->setVisible(false);

    // Create the HVC Widget
    mpHVCWidget = new HVCWidget(this);
    mpHVCWidget->setVisible(false);

    //Create the plot dock widget and hide it
    mpPlotWidgetDock = new QDockWidget(tr("Plot Variables"), this);
    mpPlotWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpPlotWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpPlotWidgetDock);
    connect(mpPlotWidgetDock, SIGNAL(visibilityChanged(bool)), this, SLOT(updatePlotActionButton(bool)));

    //Create the system parameters dock widget and hide it
    mpSystemParametersDock = new QDockWidget(tr("System Parameters"), this);
    mpSystemParametersDock->setAllowedAreas((Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea));
    addDockWidget(Qt::RightDockWidgetArea, mpSystemParametersDock);
    mpSystemParametersDock->hide();

    //Create the undo dock widget and hide it
    mpUndoWidgetDock = new QDockWidget(tr("Undo History"), this);
    mpUndoWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mpUndoWidgetDock->hide();
    addDockWidget(Qt::RightDockWidgetArea, mpUndoWidgetDock);

    //Make dock widgets that share same dock area tabified, instead of stacking them above each other
    tabifyDockWidget(mpPlotWidgetDock, mpSystemParametersDock);
    tabifyDockWidget(mpSystemParametersDock, mpUndoWidgetDock);
    tabifyDockWidget(mpUndoWidgetDock, mpPlotWidgetDock);

#ifdef USEPYTHONQT
    tabifyDockWidget(mpTerminalDock, mpPyDockWidget);
    tabifyDockWidget(mpPyDockWidget, mpMessageDock);
    tabifyDockWidget(mpMessageDock, mpTerminalDock);
#else
    tabifyDockWidget(mpTerminalDock, mpMessageDock);
#endif

    // Initialize the help message popup
    mpHelpPopup = new HelpPopUpWidget(this);

    // Set the correct position of the help popup message in the central widget
    mpCentralGridLayout->addWidget(mpHelpPopup, 1,1,1,1);
    mpCentralGridLayout->setColumnMinimumWidth(0,5);
    mpCentralGridLayout->setColumnStretch(0,0);
    mpCentralGridLayout->setColumnStretch(1,0);
    mpCentralGridLayout->setColumnStretch(2,0);
    mpCentralGridLayout->setColumnStretch(3,1);
    mpCentralGridLayout->setRowMinimumHeight(0,25);
    mpCentralGridLayout->setRowStretch(0,0);
    mpCentralGridLayout->setRowStretch(1,0);
    mpCentralGridLayout->setRowStretch(2,1);

    //Create actions, toolbars and menus
    this->createActions();
    this->createToolbars();
    this->createMenus();

    connect(mpCopyAction, SIGNAL(triggered()), mpMessageWidget, SLOT(copy()));
    connect(mpCopyAction, SIGNAL(triggered()), gpTerminalWidget->mpConsole, SLOT(copy()));

    gpTerminalWidget->loadConfig();

    mpWelcomeWidget = 0;

    // Trigger splashscreen close in one second
    QTimer::singleShot(3000, gpSplash, SLOT(close()));

    mpWelcomeWidget = new WelcomeWidget(this);

    mpCentralTabs->addTab(mpWelcomeWidget, "Welcome");
    mpCentralTabs->setTabNotClosable(0);

    this->updateRecentList();

    // Update style sheet setting after all children hasbeen created and added so that they too will be affected
    if(!gpConfig->getUseNativeStyleSheet())
    {
        setStyleSheet(gpConfig->getStyleSheet());
        setPalette(gpConfig->getPalette());
    }
    qApp->setFont(gpConfig->getFont());

    updateToolBarsToNewTab();
}


//! @brief Initializes the workspace.
//! All startup events that does not involve creating the main window and its widgets/dialogs belongs here.
void MainWindow::initializeWorkspace()
{
#ifdef USEPYTHONQT
    mpPyDockWidget->runCommand(gpConfig->getInitScript());
#endif

    gpSplash->showMessage("Loading component libraries...");

    // Load HopsanGui built in secret components
    gpLibraryHandler->loadLibrary(QString(BUILTINCAFPATH) + "hidden/builtin_hidden.xml", InternalLib, Hidden);

    // Load default and user specified libraries
    QString componentPath = gpDesktopHandler->getComponentsPath();

    // Load built in default Library
#ifdef BUILTINDEFAULTCOMPONENTLIB
    gpLibraryHandler->loadLibrary(componentPath, InternalLib);
#else
    gpLibraryHandler->loadLibrary(componentPath+"defaultComponentLibrary.xml", InternalLib);
#endif

    // Load builtIn library (Container special components)
    gpLibraryHandler->loadLibrary(QString(BUILTINCAFPATH) + "visible/builtin_visible.xml", InternalLib);

    for(int i=0; i<gpConfig->getUserLibs().size(); ++i)
    {
        gpSplash->showMessage("Loading library: "+gpConfig->getUserLibs()[i]+"...");
        gpLibraryHandler->loadLibrary(gpConfig->getUserLibs()[i], gpConfig->getUserLibTypes()[i]);
    }

    // Create the plot widget, only once! :)
    gpPlotWidget = new PlotWidget(this);
    gpPlotWidget->hide();

    // Create the data explorer widget
    //! @todo does it need to exist in memory all the time?
    mpDataExplorer = new DataExplorer(this);
    mpDataExplorer->hide();

    // File association - ignore everything else and open the specified file if there is a hmf file in the argument list
    for(int i=0; i<qApp->arguments().size(); ++i)
    {
        if(qApp->arguments().at(i).endsWith(".hmf"))
        {
            mpModelHandler->closeAllModels();
            mpModelHandler->loadModel(qApp->arguments().at(i));
            return;
        }
    }

    mpLibraryWidget->adjustSize();

    gpSplash->showMessage("Finished!");

    QTimer::singleShot(1000, gpSplash, SLOT(close()));
}


//! @brief Toggles visibility of the plot widget and its dock
void MainWindow::toggleVisiblePlotWidget()
{
    // Show it if it is currently not visible
    if(mpPlotWidgetDock->isHidden())
    {
        mpPlotWidgetDock->setWidget(gpPlotWidget);
        mpPlotWidgetDock->show();
        mpPlotWidgetDock->raise();
    }
    // Else hide it
    else
    {
        mpPlotWidgetDock->hide();
    }
}


//! @brief Event triggered re-implemented method that closes the main window.
//! First all tabs (models) are closed, if the user do not push Cancel
//! (closeAllModels then returns 'false') the event is accepted and
//! the main window is closed.
//! @param event contains information of the closing operation.
void MainWindow::closeEvent(QCloseEvent *event)
{
    // Must close all open windows before closing project tabs
    //! @todo need to restore function that closes those plots beloning to a particular tab/model automatically /Peter
    gpPlotHandler->closeAllOpenWindows();

    if (mpModelHandler->closeAllModels())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }

    gpTerminalWidget->saveConfig();
    gpConfig->saveToXml();
}


//! @brief Shows the help popup message
//! @param[in] rMessage String with text message
void MainWindow::showHelpPopupMessage(const QString &rMessage)
{
    mpHelpPopup->showHelpPopupMessage(rMessage);
}


//! @brief Hides the help popup message
void MainWindow::hideHelpPopupMessage()
{
    mpHelpPopup->hide();
}


//! @brief Returns a pointer to the python scripting dock widget.
PyDockWidget *MainWindow::getPythonDock()
{
    return mpPyDockWidget;
}



//! @brief Defines the actions used by the toolbars
void MainWindow::createActions()
{
    mpNewAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-New.png"), tr("&New"), this);
    mpNewAction->setShortcut(tr("New"));
    mpNewAction->setToolTip(tr("Create New Project"));
    connect(mpNewAction, SIGNAL(triggered()), mpModelHandler, SLOT(addNewModel()));
    connect(mpNewAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpNewAction, "Create a new empty model.");

    mpOpenAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Open.png"), tr("&Open"), this);
    mpOpenAction->setShortcut(QKeySequence("Ctrl+o"));
    mpOpenAction->setToolTip(tr("Load Model File (Ctrl+O)"));
    connect(mpOpenAction, SIGNAL(triggered()), mpModelHandler, SLOT(loadModel()));
    connect(mpOpenAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpOpenAction, "Open an existing model.");

    mpSaveAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Save.png"), tr("&Save"), this);
    mpSaveAction->setShortcut(QKeySequence("Ctrl+s"));
    mpSaveAction->setToolTip(tr("Save Model File (Ctrl+S)"));
    connect(mpSaveAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpSaveAction, "Save current model.");

    mpSaveAsAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SaveAs.png"), tr("&Save As"), this);
    mpSaveAsAction->setShortcut(QKeySequence("Ctrl+Alt+s"));
    mpSaveAsAction->setToolTip(tr("Save Model File As (Ctrl+Alt+S)"));
    connect(mpSaveAsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpSaveAsAction, "Save current model as new file.");

    mpExportModelParametersAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportParameters.png"), tr("&Export Model Parameters"), this);
    mpExportModelParametersAction->setShortcut(QKeySequence("Ctrl+Alt+E"));
    mpExportModelParametersAction->setToolTip(tr("Export Model Parameters (Ctrl+Alt+P)"));
    connect(mpExportModelParametersAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpExportModelParametersAction, "Export model parameter set to XML.");

    mpCloseAction = new QAction(this);
    mpCloseAction->setText("Close");
    mpCloseAction->setShortcut(QKeySequence("Ctrl+q"));
    connect(mpCloseAction,SIGNAL(triggered()),this,SLOT(close()));

    mpUndoAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Undo.png"), tr("&Undo"), this);
    mpUndoAction->setText("Undo");
    mpUndoAction->setShortcut(QKeySequence(tr("Ctrl+z")));
    mpUndoAction->setToolTip(tr("Undo One Step (Ctrl+Z)"));
    connect(mpUndoAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpUndoAction, "Undo last action in current model.");

    mpRedoAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Redo.png"), tr("&Redo"), this);
    mpRedoAction->setText("Redo");
    mpRedoAction->setShortcut(QKeySequence(tr("Ctrl+y")));
    mpRedoAction->setToolTip(tr("Redo One Step (Ctrl+Y)"));
    connect(mpRedoAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpRedoAction, "Redo last undone action in current model.");

    mpOpenUndoAction = new QAction(tr("&Undo History"), this);
    mpOpenUndoAction->setToolTip("Undo History (Ctrl+Shift+U)");
    connect(mpOpenUndoAction,SIGNAL(triggered()),this,SLOT(openUndoWidget()));
    mpOpenUndoAction->setShortcut(QKeySequence("Ctrl+Shift+u"));

    mpDisableUndoAction = new QAction(tr("&Disable Undo"), this);
    mpDisableUndoAction->setText("Disable Undo");
    mpDisableUndoAction->setCheckable(true);
    mpDisableUndoAction->setChecked(false);

    mpOpenSystemParametersAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SystemParameter.png"), tr("&System Parameters"), this);
    mpOpenSystemParametersAction->setToolTip("System Parameters (Ctrl+Shift+Y)");
    mpOpenSystemParametersAction->setShortcut(tr("Ctrl+Shift+y"));
    mpOpenSystemParametersAction->setCheckable(true);
    connect(mpOpenSystemParametersAction,SIGNAL(triggered()),this,SLOT(openSystemParametersWidget()));
    connect(mpOpenSystemParametersAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpOpenSystemParametersAction, "Opens the list of system parameters in current model.");

    mpOpenHvcWidgetAction = new QAction(tr("&Model Validation"), this);
    mpOpenHvcWidgetAction->setToolTip("Open the HopsanValidationConfiguration Widget");
    connect(mpOpenHvcWidgetAction, SIGNAL(triggered()), this, SLOT(openHVCWidget()));

    mpOpenDataExplorerAction = new QAction(tr("&Data Explorer"), this);
    mpOpenDataExplorerAction->setShortcut(QKeySequence("Ctrl+Shift+d"));
    mpOpenDataExplorerAction->setToolTip("Open the Data Explorer Widget (Ctrl+Shift+d)");
    connect(mpOpenDataExplorerAction, SIGNAL(triggered()), this, SLOT(openDataExplorerWidget()));

    mpCutAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Cut.png"), tr("&Cut"), this);
    mpCutAction->setShortcut(tr("Ctrl+x"));
    mpCutAction->setToolTip(tr("Cut (Ctrl+X)"));
    connect(mpCutAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpCutAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpCutAction, "Cut selected components.");

    mpCopyAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Copy.png"), tr("&Copy"), this);
    mpCopyAction->setShortcut(tr("Ctrl+c"));
    mpCopyAction->setToolTip("Copy (Ctrl+C)");
    connect(mpCopyAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpCopyAction, "Copy selected components.");

    mpPasteAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Paste.png"), tr("&Paste"), this);
    mpPasteAction->setShortcut(tr("Ctrl+v"));
    mpPasteAction->setToolTip(tr("Paste (Ctrl+V)"));
    connect(mpPasteAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpPasteAction, "Paste copied components in current model.");

    mpSimulateAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Simulate.png"), tr("&Simulate"), this);
    mpSimulateAction->setToolTip(tr("Simulate Current Project (Ctrl+Shift+S)"));
    mpSimulateAction->setShortcut(QKeySequence("Ctrl+Shift+s"));
    connect(mpSimulateAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpSimulateAction, SIGNAL(triggered()), this, SLOT(simulateKeyWasPressed()));
    mHelpPopupTextMap.insert(mpSimulateAction, "Starts a new simulation of current model.");

    mpCoSimulationAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Simulate.png"), tr("&Start Co-Simulation"), this);
    mpCoSimulationAction->setToolTip(tr("Start Co-Simulation"));
    connect(mpCoSimulationAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpOpenDebuggerAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Debug.png"), tr("&Launch Debugger"), this);
    mpOpenDebuggerAction->setToolTip(tr("Launch Debugger"));
    connect(mpOpenDebuggerAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpOpenDebuggerAction,  SIGNAL(triggered()), mpModelHandler, SLOT(launchDebugger()));
    mHelpPopupTextMap.insert(mpOpenDebuggerAction, "Open debugger dialog to examine the current model in detail.");

    mpOptimizeAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Optimize.png"), tr("&Optimize"), this);
    mpOptimizeAction->setToolTip(tr("Open Optimization Dialog (Ctrl+Shift+Z)"));
    mpOptimizeAction->setShortcut(QKeySequence("Ctrl+Shift+z"));
    connect(mpOptimizeAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpOptimizeAction, SIGNAL(triggered()), mpOptimizationDialog, SLOT(open()));
    mHelpPopupTextMap.insert(mpOptimizeAction, "Open optimization dialog to initialize numerical optimization of current model.");

    mpSensitivityAnalysisAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-SensitivityAnalysis.png"), tr("&Sensitivity Analysis"), this);
    mpSensitivityAnalysisAction->setToolTip(tr("Open Sensitivity Analysis Dialog (Ctrl+Shift+A)"));
    mpSensitivityAnalysisAction->setShortcut(QKeySequence("Ctrl+Shift+A"));
    connect(mpSensitivityAnalysisAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpSensitivityAnalysisAction, SIGNAL(triggered()), mpSensitivityAnalysisDialog, SLOT(open()));
    mHelpPopupTextMap.insert(mpSensitivityAnalysisAction, "Perform sensitivity analysis of current model.");

    mpMeasureSimulationTimeAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-MeasureSimulationTime.png"), tr("&Measure Simulation Times"), this);
    mpMeasureSimulationTimeAction->setToolTip(tr("Measure Simulation Times"));
    connect(mpMeasureSimulationTimeAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    connect(mpMeasureSimulationTimeAction, SIGNAL(triggered()), mpModelHandler, SLOT(measureSimulationTime()));
    mHelpPopupTextMap.insert(mpMeasureSimulationTimeAction, "Measure simulation time for each component in current model.");

    mpPlotAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Plot.png"), tr("&Plot Variables"), this);
    mpPlotAction->setToolTip(tr("Plot Variables (Ctrl+Shift+P)"));
    mpPlotAction->setCheckable(true);
    mpPlotAction->setShortcut(QKeySequence("Ctrl+Shift+p"));
    connect(mpPlotAction, SIGNAL(triggered()),this,SLOT(toggleVisiblePlotWidget()));
    connect(mpPlotAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpPlotAction, "Opens the list with all available plot variables from current model.");

    mpLoadLibsAction = new QAction(this);
    mpLoadLibsAction->setText("Load Libraries");
    connect(mpLoadLibsAction,SIGNAL(triggered()),gpLibraryHandler,SLOT(loadLibrary()));

    mpPropertiesAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Configure.png"), tr("&Model Properties"), this);
    mpPropertiesAction->setToolTip("Model Properties (Ctrl+Shift+M)");
    mpPropertiesAction->setShortcut(QKeySequence("Ctrl+Shift+m"));
    connect(mpPropertiesAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpPropertiesAction, "Opens a dialog with settings for the current model.");

    mpOptionsAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Options.png"), tr("&Options"), this);
    mpOptionsAction->setToolTip("Options (Ctrl+Shift+O)");
    mpOptionsAction->setShortcut(QKeySequence("Ctrl+Shift+o"));
    connect(mpOptionsAction, SIGNAL(triggered()), mpOptionsDialog, SLOT(show()));
    connect(mpOptionsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpOptionsAction, "Open options dialog to change program settings.");

    mpAnimateAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Animation.png"), tr("&Animate"), this);
    mpAnimateAction->setToolTip("Animate");
    connect(mpAnimateAction, SIGNAL(triggered()),mpModelHandler, SLOT(openAnimation()));
    connect(mpAnimateAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpAnimateAction, "Open current model in animation mode.");

    mpAlignXAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-AlignX.png"), tr("&Align Vertical (by last selected)"), this);
    mpAlignXAction->setText("Align Vertical");
    mHelpPopupTextMap.insert(mpAlignXAction, "Align selected components horizontally to last selected component.");
    connect(mpAlignXAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpAlignYAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-AlignY.png"), tr("&Align Horizontal (by last selected)"), this);
    mpAlignYAction->setText("Align Horizontal");
    mHelpPopupTextMap.insert(mpAlignYAction, "Align selected components vertically to last selected component.");
    connect(mpAlignYAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpRotateLeftAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-RotateLeft.png"), tr("&Rotate Left (Ctrl+E)"), this);
    mpRotateLeftAction->setText("Rotate Left (Ctrl+E)");
    mpRotateLeftAction->setShortcut(QKeySequence("Ctrl+E"));
    mHelpPopupTextMap.insert(mpRotateLeftAction, "Rotate selected components counter-clockwise.");
    connect(mpRotateLeftAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpRotateRightAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-RotateRight.png"), tr("&Rotate Right (Ctrl+R)"), this);
    mpRotateRightAction->setText("Rotate Right (Ctrl+R)");
    mpRotateRightAction->setShortcut(QKeySequence("Ctrl+R"));
    mHelpPopupTextMap.insert(mpRotateRightAction, "Rotate selected components clockwise.");
    connect(mpRotateRightAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpFlipHorizontalAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-FlipHorizontal.png"), tr("&Flip Horizontal"), this);
    mpFlipHorizontalAction->setText("Flip Horizontal (Ctrl+F)");
    mpFlipHorizontalAction->setShortcut(QKeySequence("Ctrl+F"));
    mHelpPopupTextMap.insert(mpFlipHorizontalAction, "Flip selected components horizontally.");
    connect(mpFlipHorizontalAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpFlipVerticalAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-FlipVertical.png"), tr("&Flip Vertical"), this);
    mpFlipVerticalAction->setText("Flip Vertical (Ctrl+D");
    mpFlipVerticalAction->setShortcut(QKeySequence("Ctrl+D"));
    mHelpPopupTextMap.insert(mpFlipVerticalAction, "Flip selected components vertically.");
    connect(mpFlipVerticalAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpResetZoomAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Zoom100.png"), tr("&Reset Zoom (Ctrl+0)"), this);
    mpResetZoomAction->setText("Reset Zoom (Ctrl+0)");
    mpResetZoomAction->setShortcut(QKeySequence("Ctrl+0"));
    connect(mpResetZoomAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpResetZoomAction, "Reset zoom to 100%.");

    mpZoomInAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ZoomIn.png"), tr("&Zoom In (Ctrl+Plus)"), this);
    mpZoomInAction->setText("Zoom In (Ctrl+Plus)");
    mpZoomInAction->setShortcut(QKeySequence("Ctrl++"));
    connect(mpZoomInAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpZoomInAction, "Increase zoom level.");

    mpZoomOutAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ZoomOut.png"), tr("&Zoom Out (Ctrl+Minus)"), this);
    mpZoomOutAction->setText("Zoom Out (Ctrl+Minus)");
    mpZoomOutAction->setShortcut(QKeySequence("Ctrl+-"));
    connect(mpZoomOutAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpZoomOutAction, "Decrease zoom level.");

    mpCenterViewAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-CenterView.png"), tr("&Center View (Ctrl+Space)"), this);
    mpCenterViewAction->setText("Center View (Ctrl+Space)");
    mpCenterViewAction->setShortcut(QKeySequence("Ctrl+Space"));
    connect(mpCenterViewAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpCenterViewAction, "Center view in current model.");

    QIcon toggleNamesIcon;
    toggleNamesIcon.addFile(QString(ICONPATH) + "Hopsan-ToggleNames.png", QSize(), QIcon::Normal, QIcon::On);
    mpToggleNamesAction = new QAction(toggleNamesIcon, tr("&Show Component Names (Ctrl+N)"), this);
    mpToggleNamesAction->setText("Show Component Names (Ctrl+N)");
    mpToggleNamesAction->setCheckable(true);
    mpToggleNamesAction->setChecked(gpConfig->getToggleNamesButtonCheckedLastSession());
    mpToggleNamesAction->setShortcut(QKeySequence("Ctrl+n"));
    connect(mpToggleNamesAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpToggleNamesAction, "Toggle  visibility of component names for all components.");

    mpPrintAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Print.png"), tr("&Print Model"), this);
    mpPrintAction->setText("Print Model");
    connect(mpPrintAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpPrintAction, "Print current model.");

    mpExportPDFAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportPdf.png"), tr("&Export To PDF"), this);
    mpExportPDFAction->setText("Export Model to PDF");
    connect(mpExportPDFAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpExportPDFAction, "Export current model to Portable Document Format (PDF).");

    mpExportPNGAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportPng.png"), tr("&Export To PNG"), this);
    mpExportPNGAction->setText("Export Model to PNG");
    connect(mpExportPNGAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpExportPNGAction, "Export current model to Portable Network Graphics (PNG).");

    mpImportFMUAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ImportFmu.png"), tr("Import Functional Mock-up Unit (FMU)"), this);
    mHelpPopupTextMap.insert(mpImportFMUAction, "Import Functional Mock-up Unit (FMU).");
    connect(mpImportFMUAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToSimulinkAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportSimulink.png"), tr("Export to Simulink S-function Source Files"), this);
    mHelpPopupTextMap.insert(mpExportToSimulinkAction, "Export model to Simulink S-function.");
    connect(mpExportToSimulinkAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToSimulinkCoSimAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportSimulinkCoSim.png"), tr("Export to Simulink Co-Simulation S-function Source Files"), this);
    mHelpPopupTextMap.insert(mpExportToSimulinkCoSimAction, "Export model Simulink S-function for co-simulation (under development).");
    connect(mpExportToSimulinkCoSimAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMUAction = new QAction(tr("FMU for Model Exchange"), this);
    mHelpPopupTextMap.insert(mpExportToFMUAction, "FMU for Model Exchange");
    connect(mpExportToFMUAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMUActionCoSim = new QAction(tr("FMU for Co-Simulation"), this);
    mHelpPopupTextMap.insert(mpExportToFMUActionCoSim, "FMU for Co-Simulation");
    connect(mpExportToFMUActionCoSim, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpExportToFMUMenu = new QMenu("Export to Functional Mock-Up Interface (FMI)");
    mpExportToFMUMenu->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ExportFmu.png"));
    mpExportToFMUMenu->addAction(mpExportToFMUAction);
    mpExportToFMUMenu->addAction(mpExportToFMUActionCoSim);

    mpExportToLabviewAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ExportSIT.png"), tr("Export to LabVIEW/SIT"), this);
    mHelpPopupTextMap.insert(mpExportToLabviewAction, "Export model to LabVIEW Veristand.");
    connect(mpExportToLabviewAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpLoadModelParametersAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-LoadModelParameters.png"), tr("Load Model Parameters"), this);
    mHelpPopupTextMap.insert(mpLoadModelParametersAction, "Load model parameter set from XML.");
    connect(mpLoadModelParametersAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));

    mpAboutAction = new QAction(this);
    mpAboutAction->setText("About");
    connect(mpAboutAction, SIGNAL(triggered()), mpAboutDialog, SLOT(open()));
    connect(mpAboutDialog->timer, SIGNAL(timeout()), mpAboutDialog, SLOT(update()));

    mpIssueTrackerAction = new QAction(this);
    mpIssueTrackerAction->setText("Issue Tracker");
    connect(mpIssueTrackerAction, SIGNAL(triggered()), this, SLOT(openIssueTrackerDialog()));

    mpHelpAction = new QAction(this);
    mpHelpAction->setText("User Guide");
    connect(mpHelpAction, SIGNAL(triggered()), mpHelpDialog, SLOT(open()));

    mpReleaseNotesAction = new QAction(this);
    mpReleaseNotesAction->setText("Release Notes");
    connect(mpReleaseNotesAction, SIGNAL(triggered()), this, SLOT(showReleaseNotes()));

    mpNewVersionsAction = new QAction(this);
    mpNewVersionsAction->setText("Check For New Versions");
    connect(mpNewVersionsAction, SIGNAL(triggered()), this, SLOT(openArchiveURL()));
    mpWebsiteAction = new QAction(this);
    mpWebsiteAction->setText("Open Hopsan Website");
    connect(mpWebsiteAction, SIGNAL(triggered()), this, SLOT(openHopsanURL()));

    //! @todo Check for new version could probably work in a better way...
    mpNewVersionsAction = new QAction(this);
    mpNewVersionsAction->setText("Check For New Versions");
    connect(mpNewVersionsAction, SIGNAL(triggered()), this, SLOT(openArchiveURL()));

    QIcon togglePortsIcon;
    togglePortsIcon.addFile(QString(ICONPATH) + "Hopsan-TogglePorts.png", QSize(), QIcon::Normal, QIcon::On);
    mpTogglePortsAction = new QAction(togglePortsIcon, tr("&Show Unconnected Ports (Ctrl+T)"), this);
    mpTogglePortsAction->setText("Show Unconnected Ports (Ctrl+T)");
    mpTogglePortsAction->setCheckable(true);
    mpTogglePortsAction->setChecked(gpConfig->getTogglePortsButtonCheckedLastSession());
    mpTogglePortsAction->setShortcut(QKeySequence("Ctrl+t"));
    connect(mpTogglePortsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpTogglePortsAction, "Toggle visibility of unconnected ports.");

    QIcon toggleSignalsIcon;
    toggleSignalsIcon.addFile(QString(ICONPATH) + "Hopsan-ToggleSignal.png", QSize(), QIcon::Normal, QIcon::On);
    mpToggleSignalsAction = new QAction(toggleSignalsIcon, tr("&Show Signal Components"), this);
    mpToggleSignalsAction->setText("Show Signal Components");
    mpToggleSignalsAction->setCheckable(true);
    mpToggleSignalsAction->setChecked(true);      //! @todo Shall depend on gpConfig setting
    connect(mpToggleSignalsAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpToggleSignalsAction, "Toggle signal components visibility.");

    mpDebug1Action = new QAction(this);
    mpDebug1Action->setShortcut(QKeySequence("Ctrl+D+1"));
    this->addAction(mpDebug1Action);
    connect(mpDebug1Action, SIGNAL(triggered()), mpModelHandler, SLOT(launchDebugger()));

    mpDebug2Action = new QAction(this);
    mpDebug2Action->setShortcut(QKeySequence("Ctrl+D+2"));
    this->addAction(mpDebug2Action);
    //connect(mpDebug2Action, SIGNAL(triggered()), mpModelHandler, SLOT(simulateAllOpenModelsWithoutSplit()));

    mpShowLossesAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-Losses.png"), tr("Calculate Losses"), this);
    mpShowLossesAction->setShortcut(QKeySequence("Ctrl+L"));
    mpShowLossesAction->setCheckable(true);
    this->addAction(mpShowLossesAction);
    connect(mpShowLossesAction, SIGNAL(triggered(bool)), mpModelHandler, SLOT(showLosses(bool)));
    connect(mpShowLossesAction, SIGNAL(hovered()), this, SLOT(showToolBarHelpPopup()));
    mHelpPopupTextMap.insert(mpShowLossesAction, "Show energy or power losses from last simulation.");

    mpToggleHideAllDockAreasAction = new QAction(QIcon(QString(ICONPATH) + "Hopsan-ShowHideDockAreas.png"), tr("Hide/Show dock areas"), this);
    mpToggleHideAllDockAreasAction->setCheckable(true);
    mpToggleHideAllDockAreasAction->setChecked(true);
    connect(mpToggleHideAllDockAreasAction, SIGNAL(toggled(bool)), this, SLOT(toggleHideShowDockAreas(bool)));

    mpSimulationTimeEdit = new SimulationTimeEdit(this);
    connect(mpSimulationTimeEdit, SIGNAL(mouseEnterEvent()), this, SLOT(showToolBarHelpPopup()));
}


//! @brief Creates the menus
void MainWindow::createMenus()
{
    // Create the menubar
    mpMenuBar = new QMenuBar();
    mpMenuBar->setGeometry(QRect(0,0,800,25));

    // Create the main menues
    mpFileMenu = new QMenu(mpMenuBar);
    mpFileMenu->setTitle(tr("&File"));

    mpRecentMenu = new QMenu(this);
    mpRecentMenu->setTitle(tr("&Recent Models"));

    mpNewMenu = new QMenu(mpMenuBar);
    mpNewMenu->setTitle(tr("&New"));

    mpSimulationMenu = new QMenu(mpMenuBar);
    mpSimulationMenu->setTitle(tr("&Simulation"));

    mpEditMenu = new QMenu(mpMenuBar);
    mpEditMenu->setTitle(tr("&Edit"));

    mpViewMenu = new QMenu(mpMenuBar);
    mpViewMenu->setTitle(tr("&View"));

    mpToolsMenu = new QMenu(mpMenuBar);
    mpToolsMenu->setTitle(tr("&Tools"));

    mpImportMenu = new QMenu(mpMenuBar);
    mpImportMenu->setTitle(tr("&Import"));

    mpExportMenu = new QMenu(mpMenuBar);
    mpExportMenu->setTitle(tr("&Export"));

    mpHelpMenu = new QMenu(mpMenuBar);
    mpHelpMenu->setTitle(tr("&Help"));

    this->setMenuBar(mpMenuBar);

    // Create sub menues for the help menue
    mpExamplesMenu = new QMenu("Example Models");
    QDir exampleModelsDir(gpDesktopHandler->getMainPath()+"Models/Example Models/");
    buildModelActionsMenu(mpExamplesMenu, exampleModelsDir);

    mpTestModelsMenu = new QMenu("Test Models");
    QDir testModelsDir(gpDesktopHandler->getMainPath()+"Models/Component Test/");
    buildModelActionsMenu(mpTestModelsMenu, testModelsDir);

    // Add the actionbuttons to the menues
    mpNewAction->setText("Project");
    mpNewMenu->addAction(mpNewAction);

    mpFileMenu->addAction(mpNewMenu->menuAction());
    mpFileMenu->addAction(mpOpenAction);
    mpFileMenu->addAction(mpSaveAction);
    mpFileMenu->addAction(mpSaveAsAction);
    mpFileMenu->addMenu(mpRecentMenu);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpPrintAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpLoadLibsAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpPropertiesAction);
    mpFileMenu->addAction(mpOpenSystemParametersAction);
    mpFileMenu->addSeparator();
    mpFileMenu->addAction(mpCloseAction);

    mpSimulationMenu->addAction(mpSimulateAction);
    mpSimulationMenu->addAction(mpOpenDebuggerAction);
#ifdef DEVELOPMENT
    mpSimulationMenu->addAction(mpCoSimulationAction);
#endif
    mpSimulationMenu->addAction(mpAnimateAction);
    mpSimulationMenu->addAction(mpMeasureSimulationTimeAction);
    mpSimulationMenu->addAction(mpOptimizeAction);
    mpSimulationMenu->addAction(mpSensitivityAnalysisAction);
    mpSimulationMenu->addAction(mpPlotAction);
    mpSimulationMenu->addAction(mpShowLossesAction);

    mpEditMenu->addAction(mpUndoAction);
    mpEditMenu->addAction(mpRedoAction);
    mpEditMenu->addAction(mpOpenUndoAction);
    mpEditMenu->addAction(mpDisableUndoAction);
    mpEditMenu->addSeparator();
    mpEditMenu->addAction(mpCopyAction);
    mpEditMenu->addAction(mpCutAction);
    mpEditMenu->addAction(mpPasteAction);

    //The View menu shall be alphabetically sorted!
    mpViewMenu->addAction(mpToggleNamesAction);
    mpViewMenu->addAction(mpTogglePortsAction);
    mpViewMenu->addAction(mpToggleSignalsAction);
    mpViewMenu->addSeparator();
    mpViewMenu->addAction(mpLibDock->toggleViewAction());
    mpViewMenu->addAction(mpEditToolBar->toggleViewAction());
    mpViewMenu->addAction(mpFileToolBar->toggleViewAction());
    mpViewMenu->addAction(mpTerminalDock->toggleViewAction());
    mpViewMenu->addAction(mpConnectivityToolBar->toggleViewAction());
    mpViewMenu->addAction(mpMessageDock->toggleViewAction());
#ifdef USEPYTHONQT
    mpViewMenu->addAction(mpPyDockWidget->toggleViewAction());
#endif
    mpViewMenu->addAction(mpSimToolBar->toggleViewAction());

    mpToolsMenu->addAction(mpOptionsAction);
    mpToolsMenu->addAction(mpOpenSystemParametersAction);
    mpToolsMenu->addAction(mpOpenHvcWidgetAction);
    mpToolsMenu->addAction(mpOpenDataExplorerAction);

    mpImportMenu->addAction(mpLoadModelParametersAction);
    mpImportMenu->addSeparator();
    mpImportMenu->addAction(mpImportFMUAction);

    mpExportMenu->addAction(mpExportModelParametersAction);
    mpExportMenu->addSeparator();
    mpExportMenu->addAction(mpExportToSimulinkAction);
    mpExportMenu->addAction(mpExportToLabviewAction);
    mpExportMenu->addMenu(mpExportToFMUMenu);
#ifdef DEVELOPMENT
    mpExportMenu->addAction(mpExportToSimulinkCoSimAction);
#endif
    mpExportMenu->addSeparator();
    mpExportMenu->addAction(mpExportPDFAction);
    mpExportMenu->addAction(mpExportPNGAction);

    mpHelpMenu->addAction(mpHelpAction);
    mpHelpMenu->addAction(mpReleaseNotesAction);
    mpHelpMenu->addMenu(mpExamplesMenu);
    mpHelpMenu->addMenu(mpTestModelsMenu);
    mpHelpMenu->addAction(mpIssueTrackerAction);
    mpHelpMenu->addAction(mpWebsiteAction);
    mpHelpMenu->addAction(mpNewVersionsAction);
    mpHelpMenu->addAction(mpAboutAction);

    mpMenuBar->addAction(mpFileMenu->menuAction());
    mpMenuBar->addAction(mpEditMenu->menuAction());
    mpMenuBar->addAction(mpToolsMenu->menuAction());
    mpMenuBar->addAction(mpSimulationMenu->menuAction());
    mpMenuBar->addAction(mpImportMenu->menuAction());
    mpMenuBar->addAction(mpExportMenu->menuAction());
    mpMenuBar->addAction(mpViewMenu->menuAction());
    mpMenuBar->addAction(mpHelpMenu->menuAction());
}

//! @brief Creates the toolbars
void MainWindow::createToolbars()
{
    //File toolbar, contains all file handling stuff (open, save etc)
    mpFileToolBar = addToolBar(tr("File Toolbar"));
    mpFileToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpFileToolBar->setAttribute(Qt::WA_MouseTracking);
    mpFileToolBar->addAction(mpNewAction);
    mpFileToolBar->addAction(mpOpenAction);
    mpFileToolBar->addAction(mpSaveAction);
    mpFileToolBar->addAction(mpSaveAsAction);
    mpFileToolBar->addAction(mpPrintAction);

    mpConnectivityToolBar = addToolBar(tr("Import/Export Toolbar)"));
    mpConnectivityToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpConnectivityToolBar->setAttribute(Qt::WA_MouseTracking);
    mpConnectivityToolBar->addAction(mpExportModelParametersAction);
    mpConnectivityToolBar->addAction(mpLoadModelParametersAction);
    mpConnectivityToolBar->addSeparator();
    mpConnectivityToolBar->addAction(mpExportPDFAction);
    mpConnectivityToolBar->addAction(mpExportPNGAction);
    mpConnectivityToolBar->addSeparator();
    mpConnectivityToolBar->addAction(mpExportToSimulinkAction);
#ifdef DEVELOPMENT
    mpConnectivityToolBar->addAction(mpExportToSimulinkCoSimAction);
#endif
    mpConnectivityToolBar->addAction(mpExportToLabviewAction);
    mpExportToFMUMenuButton = new QToolButton(this);
    mpExportToFMUMenuButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-ExportFmu.png"));
    mpExportToFMUMenuButton->setPopupMode(QToolButton::InstantPopup);
    mpExportToFMUMenuButton->setMouseTracking(true);
    mpExportToFMUMenuButton->setMenu(mpExportToFMUMenu);
    mpConnectivityToolBar->addWidget(mpExportToFMUMenuButton);
    //mpConnectivityToolBar->addAction(mpExportToFMUAction);
    mpConnectivityToolBar->addAction(mpImportFMUAction);


    //! @note Action and menu shouldn't be here, but it doesn't work otherwise because the menus are created after the toolbars
    //mpImportMenu = new QMenu("Import");
    //mpImportMenu->addAction(mpImportFMUAction);
//    mpImportButton = new QToolButton(mpFileToolBar);
//    mpImportButton->setToolTip("Import");
//    mpImportButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Import.png"));
//    mpImportButton->setMenu(mpImportMenu);
//    mpImportButton->setPopupMode(QToolButton::InstantPopup);
    //mpFileToolBar->addWidget(mpImportButton);

//    mpExportMenu = new QMenu("Export Model");
//    mpExportMenu->addAction(mpExportToSimulinkAction);
//    mpExportMenu->addAction(mpExportToFMUAction);
//    mpExportButton = new QToolButton(mpFileToolBar);
//    mpExportButton->setToolTip("Export");
//    mpExportButton->setIcon(QIcon(QString(ICONPATH) + "Hopsan-Export.png"));
//    mpExportButton->setMenu(mpExportMenu);
//    mpExportButton->setPopupMode(QToolButton::InstantPopup);
//    mpFileToolBar->addWidget(mpExportButton);

    //Simulation toolbar, contains tools for simulationg, plotting and model preferences
    mpSimToolBar = addToolBar(tr("Simulation Toolbar"));
    mpSimToolBar->setAllowedAreas(Qt::TopToolBarArea);
    mpSimToolBar->setAttribute(Qt::WA_MouseTracking);

    mpSimToolBar->addWidget(mpSimulationTimeEdit);
    mpSimToolBar->addAction(mpSimulateAction);
    mpSimToolBar->addAction(mpOpenDebuggerAction);
#ifdef DEVELOPMENT
    mpSimToolBar->addAction(mpCoSimulationAction);
#endif
    mpSimToolBar->addAction(mpOptimizeAction);
    mpSimToolBar->addAction(mpSensitivityAnalysisAction);
    mpSimToolBar->addAction(mpPlotAction);
    mpSimToolBar->addAction(mpShowLossesAction);
    mpSimToolBar->addAction(mpAnimateAction);
    mpSimToolBar->addAction(mpMeasureSimulationTimeAction);
    mpSimToolBar->addAction(mpPropertiesAction);
    mpSimToolBar->addAction(mpOpenSystemParametersAction);

    //addToolBarBreak(Qt::TopToolBarArea);

    //Edit toolbar, contains clipboard operations, undo/redo and global options
    mpEditToolBar = new QToolBar(tr("Edit Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpEditToolBar);
    mpEditToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpEditToolBar->setAttribute(Qt::WA_MouseTracking);
    mpEditToolBar->addAction(mpCutAction);
    mpEditToolBar->addAction(mpCopyAction);
    mpEditToolBar->addAction(mpPasteAction);
    mpEditToolBar->addAction(mpUndoAction);
    mpEditToolBar->addAction(mpRedoAction);
    mpEditToolBar->addAction(mpOptionsAction);

    //View toolbar, contains all cosmetic and zooming tools
    mpViewToolBar = new QToolBar(tr("View Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpViewToolBar);
    mpViewToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::LeftToolBarArea | Qt::RightToolBarArea);
    mpViewToolBar->setAttribute(Qt::WA_MouseTracking);
    mpViewToolBar->addAction(mpCenterViewAction);
    mpViewToolBar->addAction(mpResetZoomAction);
    mpViewToolBar->addAction(mpZoomInAction);
    mpViewToolBar->addAction(mpZoomOutAction);
    mpViewToolBar->addAction(mpToggleNamesAction);
    mpViewToolBar->addAction(mpTogglePortsAction);
    mpViewToolBar->addAction(mpToggleSignalsAction);
    mpViewToolBar->addAction(mpToggleHideAllDockAreasAction);

    //Tools toolbar, contains all tools used to modify the model
    mpToolsToolBar = new QToolBar(tr("Tools Toolbar"));
    addToolBar(Qt::LeftToolBarArea, mpToolsToolBar);
    mpToolsToolBar->setAttribute(Qt::WA_MouseTracking);
    mpToolsToolBar->addAction(mpAlignXAction);
    mpToolsToolBar->addAction(mpAlignYAction);
    mpToolsToolBar->addAction(mpRotateRightAction);
    mpToolsToolBar->addAction(mpRotateLeftAction);
    mpToolsToolBar->addAction(mpFlipHorizontalAction);
    mpToolsToolBar->addAction(mpFlipVerticalAction);

    connect(mpImportFMUAction,              SIGNAL(triggered()), gpLibraryHandler,     SLOT(importFmu()));
    connect(mpExportToSimulinkAction,       SIGNAL(triggered()), mpModelHandler, SLOT(exportCurrentModelToSimulink()));
    connect(mpExportToSimulinkCoSimAction,  SIGNAL(triggered()), mpModelHandler, SLOT(exportCurrentModelToSimulinkCoSim()));
    connect(mpExportToFMUAction,            SIGNAL(triggered()), mpModelHandler, SLOT(exportCurrentModelToFMU()));
    connect(mpExportToFMUActionCoSim,            SIGNAL(triggered()), mpModelHandler, SLOT(exportCurrentModelToFMUCoSim()));
    connect(mpExportToLabviewAction,        SIGNAL(triggered()), mpModelHandler, SLOT(createLabviewWrapperFromCurrentModel()));
    connect(mpLoadModelParametersAction,    SIGNAL(triggered()), mpModelHandler, SLOT(loadModelParameters()));
}

void MainWindow::buildModelActionsMenu(QMenu *pParentMenu, QDir dir)
{
    QFileInfoList entrys = dir.entryInfoList(QStringList("*.hmf"), QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::AllDirs);
    for (int i=0; i<entrys.size(); ++i)
    {
        qDebug() << entrys[i].absolutePath() << " " << entrys[i].baseName();
        if (entrys[i].isDir())
        {
            QDir newDir(entrys[i].absoluteFilePath());
            QMenu *pMenu = new QMenu(newDir.dirName());
            pParentMenu->addMenu(pMenu);
            buildModelActionsMenu(pMenu, newDir);
        }
        else
        {
            QAction *pTempAction = new QAction(entrys[i].fileName(), this);
            pTempAction->setIcon(QIcon(QString(ICONPATH) + "hmf.ico"));
            pTempAction->setData(entrys[i].absoluteFilePath());
            pParentMenu->addAction(pTempAction);
            connect(pTempAction, SIGNAL(triggered()), this, SLOT(openModelByAction()));
        }
    }
}


//! @brief Opens the undo widget.
void MainWindow::openUndoWidget()
{
    if(!mpUndoWidgetDock->isVisible())
    {
        mpUndoWidgetDock->setWidget(mpUndoWidget);
        mpUndoWidgetDock->show();
        mpUndoWidgetDock->raise();
        mpUndoWidget->refreshList();
    }
}


//! @brief Opens the SystemParametersWidget widget.
void MainWindow::openSystemParametersWidget()
{
    if(!mpSystemParametersDock->isVisible())
    {
        mpSystemParametersDock->setWidget(mpSystemParametersWidget);
        mpSystemParametersDock->show();
        mpSystemParametersDock->raise();
        connect(mpSystemParametersDock, SIGNAL(visibilityChanged(bool)), this, SLOT(updateSystemParametersActionButton(bool)));
    }
    else
    {
        mpSystemParametersDock->hide();
    }
}


//! @brief Opens a recent model
void MainWindow::openRecentModel()
{
    QAction *action = qobject_cast<QAction *>(sender());
    qDebug() << "Trying to open " << action->text();
    if (action)
    {
        mpModelHandler->loadModel(action->text());
    }
}


void MainWindow::openHopsanURL()
{
    QDesktopServices::openUrl(QUrl(QString(HOPSANLINK)));
}


void MainWindow::openArchiveURL()
{
    QDesktopServices::openUrl(QUrl(QString(DOWNLOADLINK)));
}

void MainWindow::openIssueTrackerDialog()
{
    QMessageBox msgBox(this);
//    QSizePolicy pol;
//    pol.setHorizontalPolicy(QSizePolicy::Expanding);
//    msgBox.setSizePolicy(pol);
    QString msg = QString("To browse or read about currently known issues you should see the Hopsan project issue tracker. You can also support development by reporting new issues or add feature requests.\n\n")+
                  QString("See the Hopsan web page for more information about bug reporting and issue tracking.");

    msgBox.setWindowTitle("Hopsan Issue Tracking");
    msgBox.setText(msg);
    QPushButton *pToWeb = msgBox.addButton("To Hopsan webpage", QMessageBox::YesRole);
    msgBox.addButton(QMessageBox::Close);
    msgBox.exec();
    if (msgBox.clickedButton() == pToWeb)
    {
        this->openHopsanURL();
    }
}


//! @brief Changes the checked setting of plot widget button when plot widget is opened or closed
void MainWindow::updatePlotActionButton(bool)
{
    mpPlotAction->setChecked(mpPlotWidgetDock->isVisible() || !tabifiedDockWidgets(mpPlotWidgetDock).isEmpty());
}


//! @brief Changes the checked setting of system parameters button when the system parameter widget is opened or closed
void MainWindow::updateSystemParametersActionButton(bool)
{
    mpOpenSystemParametersAction->setChecked(mpSystemParametersDock->isVisible() || !tabifiedDockWidgets(mpSystemParametersDock).isEmpty());
}


//! @brief Shows help popup for the toolbar icon that is currently hovered by the mouse pointer
void MainWindow::showToolBarHelpPopup()
{
    // Check all tool bars to see if an action is hovered by cursor
    QCursor cursor;
    QAction *pHoveredAction = mpSimToolBar->actionAt(mpSimToolBar->mapFromGlobal(cursor.pos()));
    if(!pHoveredAction)
        pHoveredAction = mpFileToolBar->actionAt(mpFileToolBar->mapFromGlobal(cursor.pos()));
    if(!pHoveredAction)
        pHoveredAction = mpConnectivityToolBar->actionAt(mpConnectivityToolBar->mapFromGlobal(cursor.pos()));
    if(!pHoveredAction)
        pHoveredAction = mpEditToolBar->actionAt(mpEditToolBar->mapFromGlobal(cursor.pos()));
    if(!pHoveredAction)
        pHoveredAction = mpToolsToolBar->actionAt(mpToolsToolBar->mapFromGlobal(cursor.pos()));
    if(!pHoveredAction)
        pHoveredAction = mpViewToolBar->actionAt(mpViewToolBar->mapFromGlobal(cursor.pos()));

    // See if action exists in map, or if a line edit is hovered
    if(mHelpPopupTextMap.contains(pHoveredAction))
    {
        showHelpPopupMessage(mHelpPopupTextMap.find(pHoveredAction).value());
    }
    else if (mpSimulationTimeEdit->underMouse())
    {
        showHelpPopupMessage("Set simulation time (in seconds).");
    }
}


//! @brief Slot that loads an example model, based on the name of the calling action
void MainWindow::openModelByAction()
{
    QAction *pAction = qobject_cast<QAction *>(sender());
    if (pAction)
    {
        QString modelPath = pAction->data().toString();
        qDebug() << "Trying to open " << modelPath;
        mpModelHandler->loadModel(modelPath);
    }
}


//! @todo Does this function need to be in main window? (Will require more includes of mainwindow.h)
void MainWindow::openContextHelp()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action != 0)
    {
        if(action->parent() == mpSensitivityAnalysisDialog)
        {
            mpHelpDialog->open("userSensitivityAnalysis.html");
        }
        else if(action->parent() == mpModelHandler->getCurrentViewContainerObject())
        {
            mpHelpDialog->open("userEnergyLosses.html");
        }
        else if(action->parent() == mpLibraryWidget)
        {
            mpHelpDialog->open("userCustomComponents.html");
        }
        else
        {
            mpHelpDialog->open();
        }
    }
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    if(button != 0)
    {
        if(button->parent() == mpLibraryWidget)
        {
            mpHelpDialog->open("userCustomComponents.html");
        }
        else if(button->objectName() == "optimizationHelpButton")
        {
            mpHelpDialog->open("userOptimization.html");
        }
        else
        {
            mpHelpDialog->open();
        }
    }
    mpHelpDialog->centerOnScreen();
}


void MainWindow::openContextHelp(QString file)
{
    mpHelpDialog->open(file);
    mpHelpDialog->centerOnScreen();
}



void MainWindow::showReleaseNotes()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(gpDesktopHandler->getExecPath()+"../Hopsan-release-notes.txt"));
}

void MainWindow::toggleHideShowDockAreas(bool show)
{
    // Show default set of docks
    //! @todo would be nice if we would remember those opened and reshow them, but there will be odd behavior if uo hide and tehn oopen some manually and close tehm again, then triggering show will reshow them, but maybe not a big deal
    if (show)
    {
        mpLibDock->show();
        mpTerminalDock->show();
        mpMessageDock->show();
#ifdef USEPYTHONQT
        mpPyDockWidget->show();
#endif
        //mpPlotWidgetDock->show();
        //mpSystemParametersDock->hide();
    }
    else
    {
        mpLibDock->hide();
        mpTerminalDock->hide();
        mpMessageDock->hide();
#ifdef USEPYTHONQT
        mpPyDockWidget->hide();
#endif
        mpPlotWidgetDock->hide();
        mpSystemParametersDock->hide();
    }
}


//! @brief Updates the toolbar values that are tab specific when a new tab is activated
void MainWindow::updateToolBarsToNewTab()
{
    if(mpModelHandler->count() > 0 && mpModelHandler->getCurrentModel())
    {
        mpTogglePortsAction->setChecked(mpModelHandler->getCurrentModel()->getTopLevelSystemContainer()->areSubComponentPortsShown());
    }

    bool noTabs = !(mpModelHandler->count() > 0);
    mpSaveAction->setEnabled(!noTabs);
    mpExportToFMUMenuButton->setEnabled(!noTabs);
    mpSaveAsAction->setEnabled(!noTabs);
    mpExportModelParametersAction->setEnabled(!noTabs);
    mpCutAction->setEnabled(!noTabs);
    mpCopyAction->setEnabled(!noTabs);
    mpPasteAction->setEnabled(!noTabs);
    mpUndoAction->setEnabled(!noTabs);
    mpRedoAction->setEnabled(!noTabs);
    mpCenterViewAction->setEnabled(!noTabs);
    mpResetZoomAction->setEnabled(!noTabs);
    mpZoomInAction->setEnabled(!noTabs);
    mpZoomOutAction->setEnabled(!noTabs);
    mpToggleNamesAction->setEnabled(!noTabs);
    mpToggleSignalsAction->setEnabled(!noTabs);
    mpTogglePortsAction->setEnabled(!noTabs);
    mpTogglePortsAction->setEnabled(!noTabs);
    mpPrintAction->setEnabled(!noTabs);
    mpExportPDFAction->setEnabled(!noTabs);
    mpExportPNGAction->setEnabled(!noTabs);
    mpAlignXAction->setEnabled(!noTabs);
    mpAlignYAction->setEnabled(!noTabs);
    mpRotateLeftAction->setEnabled(!noTabs);
    mpRotateRightAction->setEnabled(!noTabs);
    mpFlipHorizontalAction->setEnabled(!noTabs);
    mpFlipVerticalAction->setEnabled(!noTabs);
    mpSimulationTimeEdit->setEnabled(!noTabs);
    mpSimulateAction->setEnabled(!noTabs);
    mpOpenDebuggerAction->setEnabled(!noTabs);
    mpCoSimulationAction->setEnabled(!noTabs);
    mpOptimizeAction->setEnabled(!noTabs);
    mpSensitivityAnalysisAction->setEnabled(!noTabs);
    mpMeasureSimulationTimeAction->setEnabled(!noTabs);
    mpPlotAction->setEnabled(!noTabs);
    mpShowLossesAction->setEnabled(!noTabs);
    mpAnimateAction->setEnabled(!noTabs);
    mpPropertiesAction->setEnabled(!noTabs);
    mpOpenSystemParametersAction->setEnabled(!noTabs);
    mpExportToFMUAction->setEnabled(!noTabs);
    mpExportToFMUActionCoSim->setEnabled(!noTabs);
    mpExportToLabviewAction->setEnabled(!noTabs);
    mpExportToSimulinkAction->setEnabled(!noTabs);
    mpExportToSimulinkCoSimAction->setEnabled(!noTabs);
    mpLoadModelParametersAction->setEnabled(!noTabs);
}


//! @brief Slot that calls refresh list function in undo widget. Used because undo widget cannot have slots.
void MainWindow::refreshUndoWidgetList()
{
    mpUndoWidget->refreshList();
}


//! @brief Registers a recently opened model file in the "Recent Models" list
void MainWindow::registerRecentModel(QFileInfo model)
{
    if(model.fileName() == "")
        return;

    gpConfig->addRecentModel(model.filePath());
    updateRecentList();
}

//! @brief Unregisters a model from the "Recent Models" list
void MainWindow::unRegisterRecentModel(QFileInfo model)
{
    if(model.fileName() == "")
        return;

    gpConfig->removeRecentModel(model.filePath());
    updateRecentList();
}


//! @brief Updates the "Recent Models" list
void MainWindow::updateRecentList()
{
    mpRecentMenu->clear();

    mpRecentMenu->setEnabled(!gpConfig->getRecentModels().empty());
    if(!gpConfig->getRecentModels().empty())
    {
        for(int i=0; i<gpConfig->getRecentModels().size(); ++i)
        {
            if(gpConfig->getRecentModels().at(i) != "")
            {
                QAction *tempAction;
                tempAction = mpRecentMenu->addAction(gpConfig->getRecentModels().at(i));
                tempAction->setIcon(QIcon(QString(ICONPATH) + "hmf.ico"));
                disconnect(mpRecentMenu, SIGNAL(triggered(QAction *)), mpModelHandler, SLOT(loadModel(QAction *)));    //Ugly hack to make sure connecetions are not made twice (then program would try to open model more than once...)
                connect(tempAction, SIGNAL(triggered()), this, SLOT(openRecentModel()));
            }
        }
    }

    mpWelcomeWidget->updateRecentList();
}


void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    hideHelpPopupMessage();
    QMainWindow::mouseMoveEvent(event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "Mainwindow caught keypress: " << event->key();
    QMainWindow::keyPressEvent(event);
}


OptionsDialog *MainWindow::getOptionsDialog()
{
    return mpOptionsDialog;
}


//! @brief Access function to the starttimelabel value.
//! @returns the starttime value
double MainWindow::getStartTimeFromToolBar()
{
    return mpSimulationTimeEdit->getStartTime().toDouble();
}


//! @brief Access function to the timesteplabel value.
//! @returns the timestep value
double MainWindow::getTimeStepFromToolBar()
{
    return mpSimulationTimeEdit->getTimeStep().toDouble();
}


//! @brief Access function to the finishlabel value.
//! @returns the finish value
double MainWindow::getFinishTimeFromToolBar()
{
    return mpSimulationTimeEdit->getStopTime().toDouble();
}

void MainWindow::simulateKeyWasPressed()
{
    mpSimulationTimeEdit->clearFocus();
    emit simulateKeyPressed();
}

void MainWindow::openHVCWidget()
{
    mpHVCWidget->setVisible(true);
}

void MainWindow::openDataExplorerWidget()
{
    mpDataExplorer->setVisible(true);
}


SimulationTimeEdit::SimulationTimeEdit(QWidget *pParent) :
    QWidget(pParent)
{
    mpStartTimeLineEdit = new QLineEdit("0.0", this);
    mpStartTimeLineEdit->setValidator(new QDoubleValidator(MINSTARTTIME, MAXSTARTTIME, 1000));
    mpStartTimeLineEdit->setToolTip("Set start time for simulation");
    mpStartTimeLineEdit->setMaximumWidth(70);
    mpStartTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    mpTimeStepLineEdit = new QLineEdit("0.001", this);
    mpTimeStepLineEdit->setValidator(new QDoubleValidator(MINTIMESTEP, MAXTIMESTEP, 1000));
    mpTimeStepLineEdit->setToolTip("Set step time for simulation");
    mpTimeStepLineEdit->setMaximumWidth(70);
    mpTimeStepLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    mpStopTimeLineEdit = new QLineEdit("10.0", this);
    mpStopTimeLineEdit->setValidator(new QDoubleValidator(MINSTOPTIME, MAXSTOPTIME, 1000));
    mpStopTimeLineEdit->setToolTip("Set stop time for simulation");
    mpStopTimeLineEdit->setMaximumWidth(70);
    mpStopTimeLineEdit->setAlignment(Qt::AlignVCenter | Qt::AlignCenter);

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(mpStartTimeLineEdit);
    pLayout->addWidget(new QLabel(" :: "));
    pLayout->addWidget(mpTimeStepLineEdit);
    pLayout->addWidget(new QLabel(" :: "));
    pLayout->addWidget(mpStopTimeLineEdit);

    setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed));

    connect(mpStartTimeLineEdit, SIGNAL(editingFinished()), this, SLOT(emitSimTime()));
    connect(mpTimeStepLineEdit, SIGNAL(editingFinished()), this, SLOT(emitSimTime()));
    connect(mpStopTimeLineEdit, SIGNAL(editingFinished()), this, SLOT(emitSimTime()));

    installEventFilter(this);
}

void SimulationTimeEdit::getSimulationTime(QString &rStartTime, QString &rTimeStep, QString &rStopTime) const
{
    rStartTime = mpStartTimeLineEdit->text();
    rTimeStep = mpTimeStepLineEdit->text();
    rStopTime = mpStopTimeLineEdit->text();
}

QString SimulationTimeEdit::getStartTime() const
{
    return mpStartTimeLineEdit->text();
}

QString SimulationTimeEdit::getTimeStep() const
{
    return mpTimeStepLineEdit->text();
}

QString SimulationTimeEdit::getStopTime() const
{
    return mpStopTimeLineEdit->text();
}

void SimulationTimeEdit::displaySimulationTime(const QString startTime, const QString timeStep, const QString stopTime)
{
    displayStartTime(startTime);
    displayTimeStep(timeStep);
    dispalyStopTime(stopTime);
}

bool SimulationTimeEdit::eventFilter(QObject *object, QEvent *event)
{
    if( object==this && (event->type()==QEvent::Enter) )
    {
        emit mouseEnterEvent();
    }
    return QWidget::eventFilter(object, event);
}

void SimulationTimeEdit::displayStartTime(const QString startTime)
{
    mpStartTimeLineEdit->setText(startTime);
}

void SimulationTimeEdit::displayTimeStep(const QString timeStep)
{
    mpTimeStepLineEdit->setText(timeStep);
}

void SimulationTimeEdit::dispalyStopTime(const QString stopTime)
{
    mpStopTimeLineEdit->setText(stopTime);
}

void SimulationTimeEdit::clearFocus()
{
    if (mpStartTimeLineEdit->hasFocus())
    {
        mpStartTimeLineEdit->clearFocus();
    }
    if (mpStopTimeLineEdit->hasFocus())
    {
        mpStopTimeLineEdit->clearFocus();
    }
    if (mpTimeStepLineEdit->hasFocus())
    {
        mpTimeStepLineEdit->clearFocus();
    }
}



void SimulationTimeEdit::emitSimTime()
{
    emit simulationTimeChanged(mpStartTimeLineEdit->text(), mpTimeStepLineEdit->text(), mpStopTimeLineEdit->text());
}
