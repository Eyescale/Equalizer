
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *               2013, David Steiner  <steiner@ifi.uzh.ch>
 */

#include "mainWindow.h"

#include "controller.h"
#include "connectDialog.h"

#include "ivs/src/TransferFunctionEditor.h"
#include "ivs/src/TransferFunction.h"

#include "settings.h"

#include <volVis/EQ/guiPackets.h> // VOL_VIS_GUI_MAX_PATH_LEN

#include <QtGui/QtGui>
#include <QtCore/QDebug>

#include <QtGui/QAction>
#include <QtGui/QMenu>

#include <QtGui/QShortcut>

#include <QtGui/QTextEdit>
 
namespace massVolGUI
{

MainWindow::MainWindow( QWidget *_parent )
    : RecentFiles( _parent, RecentFiles::Files, true )
    , _fileMenu( 0 )
    , _editMenu( 0 )
    , _helpMenu( 0 )
    , _fileToolBar( 0 )
    , _linkAct( 0 )
    , _openAct( 0 )
    , _tfAct( 0 )
    , _aboutAct( 0 )
    , _quitAct( 0 )
    , _controller( 0 )
    , _tf( 0 )
    , _connectDialog( 0 )
{
    _tf = new ivs::TransferFunctionPair();
    EQASSERT( _tf );
    _tf->first.init(  256, ivs::tf::RGBA8 ); // rgba 
    _tf->second.init( 256, ivs::tf::RGBA8 ); //  sda

    _controller = new Controller( _tf );
    EQASSERT( _controller );

    setWindowTitleBase(tr( "volVisGUI" ));

    QTextEdit* textEdit = new QTextEdit( this );
    textEdit->setEnabled( false );
    setCentralWidget( textEdit );

    _createActions();
    _createMenus();
    _createToolBars();
    _createStatusBar();
    _createDockWindows();

    setUnifiedTitleAndToolBarOnMac( true );

    _connectDialog = new ConnectDialog( this );
    _connectDialog->setObjectName( QString::fromUtf8( "_connectDialog" ));

    _readSettings();
}

MainWindow::~MainWindow()
{
    delete _tf;         _tf         = 0;
    delete _controller; _controller = 0;
}


/**
 *  Saves windows' and widgets' positions and sizes when application exits.
 */
void MainWindow::closeEvent( QCloseEvent *arg )
{
    _saveSettings();
    QMainWindow::closeEvent( arg );
}


void MainWindow::_saveSettings()
{
     QSettings settings;
     settings.setValue( "geometry", saveGeometry() );
     settings.setValue( "windowState", saveState() );

     RecentFiles::_saveSettings();
}


/**
 *  Reads windows' and widgets' positions and sizes when application starts.
 */
void MainWindow::_readSettings()
{
    QSettings settings;

    if( settings.contains( "geometry" ))
        restoreGeometry( settings.value("geometry").toByteArray() );

    if( settings.contains( "windowState" ))
        restoreState( settings.value("windowState").toByteArray() );
}


/**
 * Shows some information about the application.
 *
 */
void MainWindow::_about()
{
   QMessageBox::about(this, tr("About volVisGUI"),
            tr("The <b>volVisGUI</b> program is ..."));
}


void MainWindow::_createActions()
{
// connect to an application
    _linkAct = new QAction( QIcon(":/link.png"), tr("&Connect..."), this );  Q_ASSERT( _linkAct );
    _linkAct->setShortcut(tr("Ctrl+C"));
    connect( _linkAct, SIGNAL( triggered( )), this, SLOT( _connect( )));

// open a file
    _openAct = new QAction( QIcon(":/open.png"), tr("&Open..."), this );  Q_ASSERT( _openAct );
    _openAct->setShortcut(tr("Ctrl+O"));
    connect( _openAct, SIGNAL( triggered( )), this, SLOT( _open( )));

// transfer function editor
    _tfAct = new QAction( QIcon(":/tune.png"), tr("Edit &TF"), this );  Q_ASSERT( _tfAct );
    _tfAct->setShortcut(tr("Ctrl+T"));
    _tfAct->setEnabled( true );
    connect( _tfAct, SIGNAL( triggered( )), this, SLOT( _adjustTF()));

// quit app
    _quitAct = new QAction( tr("&Quit"), this );   Q_ASSERT( _quitAct );
    _quitAct->setShortcuts( QKeySequence::Quit );
    _quitAct->setStatusTip( tr("Quit the application") );
    connect(_quitAct, SIGNAL(triggered()), this, SLOT( close()));

// about
    _aboutAct = new QAction( tr("&About"), this );   Q_ASSERT( _aboutAct );
    _aboutAct->setStatusTip( tr("Show the application's About box") );
    connect(_aboutAct, SIGNAL(triggered()), this, SLOT( _about()));


// recent directories actions
    RecentFiles::_createActions();
}


//============== Menus, Docks, Toolbars ==============


void MainWindow::_createMenus()
{
    Q_ASSERT( menuBar() );
// file menu + recent dirs
    _fileMenu = menuBar()->addMenu(tr("&File"));   Q_ASSERT( _fileMenu );
    _fileMenu->addAction( _linkAct );
    _fileMenu->addAction( _openAct );


    RecentFiles::_createMenus( _fileMenu );
    connect( this, SIGNAL( openRecentAction( const QString& )),
             this, SLOT(          _loadFile( const QString& )));

    _fileMenu->addSeparator();
    _fileMenu->addAction( _quitAct );

// edit menu
    _editMenu = menuBar()->addMenu(tr("&Edit"));   Q_ASSERT( _editMenu );
    _editMenu->addAction( _tfAct   );

    menuBar()->addSeparator();

// help menu
    _helpMenu = menuBar()->addMenu(tr("&Help"));   Q_ASSERT( _helpMenu );
    _helpMenu->addAction( _aboutAct );
}


void MainWindow::_createToolBars()
{
    _fileToolBar = addToolBar( tr("File") );   Q_ASSERT( _fileToolBar );
    _fileToolBar->setObjectName( QString::fromUtf8( "_fileToolBar" ));
    _fileToolBar->addAction( _linkAct );
    _fileToolBar->addAction( _openAct );
    _fileToolBar->addAction( _tfAct   );
}


void MainWindow::_createStatusBar()
{
    Q_ASSERT( statusBar() );

    statusBar()->showMessage(tr("Ready") );
}


void MainWindow::_createDockWindows()
{
}



/**
 *  Open file dialog
 */
void MainWindow::_open()
{
    QFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setDirectory( getCurrentName( ));
    if( !dialog.exec( ))
        return;

    QStringList files = dialog.selectedFiles();
    if( files.isEmpty( ) || files.first().isEmpty( ))
        return;

    _loadFile( files.first( ));
}


void MainWindow::_loadFile( const QString& fileName )
{
    if( fileName.length() > VOL_VIS_GUI_MAX_PATH_LEN-1 )
    {
        QMessageBox::warning( this, QString( "Exception" ), 
                                    QString( "File path is too long (%1 characters max)" ).arg( VOL_VIS_GUI_MAX_PATH_LEN-1 ),
                                    QMessageBox::Ok, QMessageBox::Ok );
        return;
    }
    _controller->loadFile( fileName.toStdString() );
}


/**
 *  Call TF Editor.
 */
void MainWindow::_adjustTF()
{
    ivs::TransferFunctionEditor tfEditor( this, this );
    tfEditor.setTransferFunction( _tf );
    tfEditor.exec();
}


void MainWindow::updateTransferFunction( )
{
    static int i = 0;
    i++;
//    _controller->setX( i );
    _controller->updateTF();
}

void MainWindow::_connect()
{
    if( _controller->isConnected( ))
    {
        _controller->disconnect();
        return;
    }

    int result = _connectDialog->exec();
    if( result == QDialog::Accepted )
    {
        QString host = _connectDialog->getHost();
        QString port = _connectDialog->getPort();
        if( !_controller->connect( host.toStdString(), port.toShort( )))
        {
            QMessageBox::warning( this, QString( "Exception" ), 
                                        QString( "Failed to connect to %1:%2" ).arg( host, port ), 
                                        QMessageBox::Ok, QMessageBox::Ok );
        }
    }
}



} //namespace GUI

