
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */


#ifndef MAXUS_RECENT_FILES_H
#define MAXUS_RECENT_FILES_H

#include <QtGui/QMainWindow>
#include <QtCore/QString>
#include <QtCore/QStringList>

QT_FORWARD_DECLARE_CLASS( QMenu   )
QT_FORWARD_DECLARE_CLASS( QAction )

namespace massVolGUI
{

/**
 *  Implementation of the QMainWindow that supports recent files.
 */
class RecentFiles : public QMainWindow
{
    Q_OBJECT
public:
    enum RecentFilesType { Dirs  = 0, Files = 1	};

    RecentFiles( QWidget *parent = 0, RecentFilesType t = Dirs, bool adjustTitle = false );
    ~RecentFiles();

    /**
     *  If adjustTitle is true, window title will be adjusted automatically
     */
    void setWindowTitleBase( const QString& titleBase );

    /**
     *  When a directory gets selected from "open" or from recent dirs. This 
     *  function will update recent dir menu and also store it in app's system 
     *  settings.
     */
    void setCurrentName( const QString &name );

    /* last opened file / folder */
    const QString getCurrentName();

    /**
     *  Returns item's stripped name ("/home/user/desktop" -> "desktop" )
     */
    QString strippedName( const QString &fullName );

signals:
    /**
     *  This function will be called when one of the recent files are selected from File menu
     */
    void openRecentAction( const QString& );

private slots:
    /**
     *  Called when any of recent files are picked from the menu. 
     *  Decodes file name and calls openRecentAction function.
     */
    void _openRecent();

protected:
    void _createActions();
    void _createMenus( QMenu* fileMenu );

    void _readSettings();
    void _saveSettings();

private:

    void _updateWindowTitle();

    /**
     *  Updates list of recent items in the File menu.
     */
    void _updateRecentActions();

    enum { _maxRecentItems = 5 };

    QAction *_separatorAct;

    QAction *_recentActs[ _maxRecentItems ];

    QStringList     _names;
    RecentFilesType _type;
    bool            _changed;
    bool            _adjustTitle;
    QString         _titleBase;
};

} //namespace massVolGUI

#endif //MAXUS_RECENT_FILES_H

