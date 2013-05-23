
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */


#ifndef MASS_VOL__GUI_MAINWINDOW_H
#define MASS_VOL__GUI_MAINWINDOW_H

#include <msv/types/types.h> // byte

#include "recentFiles.h"
#include <QtCore/QMutex>
#include <QtCore/QModelIndex>

QT_FORWARD_DECLARE_CLASS( QGraphicsScene )
QT_FORWARD_DECLARE_CLASS( QGraphicsView )
QT_FORWARD_DECLARE_CLASS( QLabel )
QT_FORWARD_DECLARE_CLASS( QSlider )
QT_FORWARD_DECLARE_CLASS( QSplitter )

QT_FORWARD_DECLARE_CLASS( QAction )
QT_FORWARD_DECLARE_CLASS( QMenu )

QT_FORWARD_DECLARE_CLASS( QShortcut )
QT_FORWARD_DECLARE_CLASS( QTableView )


namespace ivs
{
    struct TransferFunctionPair;
}


namespace massVolGUI
{

class ConnectDialog;
class Controller;

class MainWindow : public RecentFiles
{
    Q_OBJECT
public:
    MainWindow( QWidget *parent = 0 );
    ~MainWindow();


    void updateTransferFunction();

signals:
    void updateRecentActions();

private slots:
    void _about();
    void _open();

    void _loadFile( const QString& fileName );

    void _connect();
    void _adjustTF();

private:

    QString _lastSavedFileName1;
    QString _lastSavedFileName2;

// GUI create functions
    void _createActions();
    void _createMenus();
    void _createToolBars();
    void _createStatusBar();
    void _createDockWindows();

// menus
    QMenu    *_fileMenu;
    QMenu    *_editMenu;
    QMenu    *_helpMenu;
    QToolBar *_fileToolBar;

// actions
    QAction *_linkAct;
    QAction *_openAct;
    QAction *_tfAct;
    QAction *_aboutAct;
    QAction *_quitAct;

    void _readSettings();
    void _saveSettings();

// QT Events
    void closeEvent( QCloseEvent *event );

// Data
    Controller *_controller;

// TF
    ivs::TransferFunctionPair *_tf;

// Dialogs
    ConnectDialog *_connectDialog;
};

} //namespace massVolGUI

#endif //MASS_VOL__GUI_MAINWINDOW_H
