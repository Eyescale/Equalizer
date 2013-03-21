
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */


#include "recentFiles.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtGui/QMenu>
#include <QtCore/QSettings>
#include <QtCore/QDebug>


namespace massVolGUI
{

RecentFiles::RecentFiles( QWidget *_parent, RecentFilesType t, bool adjustTitle )
    : QMainWindow( _parent )
    , _separatorAct( 0 )
    , _type( t )
    , _changed( false )
    , _adjustTitle( adjustTitle )
{
    for( int i = 0; i < _maxRecentItems; ++i )
        _recentActs[i] = 0;

    _readSettings();
}


RecentFiles::~RecentFiles()
{
    if( _changed )
        _saveSettings();

    delete _separatorAct;
    _separatorAct = 0;

    for( int i = 0; i < _maxRecentItems; ++i )
        if( _recentActs[i] )
        {
            delete _recentActs[i];
            _recentActs[i] = 0;
        }
}


void RecentFiles::_readSettings()
{
    _names.clear();

    QSettings settings;
    if( settings.contains( "recentNamesList" ))
        _names = settings.value( "recentNamesList" ).toStringList();
    _changed = false;
}


void RecentFiles::_saveSettings()
{
    QSettings settings;
    settings.setValue( "recentNamesList", _names );
    _changed = false;
}


void RecentFiles::_createActions()
{
    for( int i = 0; i < _maxRecentItems; ++i )
    {
        _recentActs[i] = new QAction( this );
        Q_ASSERT( _recentActs[i] );
        _recentActs[i]->setVisible( false );
        connect( _recentActs[i], SIGNAL(triggered()), this, SLOT( _openRecent()));
    }
}


void RecentFiles::_openRecent()
{
    QAction *action = qobject_cast<QAction *>( sender( ));
    if( action )
    {
        const QString name = action->data().toString( );
        setCurrentName( name );
        emit openRecentAction( name );
    }
}


void RecentFiles::_createMenus( QMenu* fileMenu )
{
    Q_ASSERT( fileMenu );
    _separatorAct = fileMenu->addSeparator();

    for (int i = 0; i < _maxRecentItems; ++i)
        fileMenu->addAction( _recentActs[i] );

    _updateRecentActions();
}


QString RecentFiles::strippedName( const QString &fullName )
{
    if( _type == Dirs )
        return QDir( fullName ).dirName();
    else // Files
        return QFile( fullName ).fileName();
}


void RecentFiles::_updateRecentActions()
{
    const int numRecentItems = qMin( _names.size(), (int)_maxRecentItems );

    for( int i = 0; i < numRecentItems; ++i )
    {
        Q_ASSERT( _recentActs[i] );
        QString text = tr("&%1 %2").arg(i + 1).arg( strippedName( _names[i] ));
        _recentActs[i]->setText(     text     );
        _recentActs[i]->setData(    _names[i] );
        _recentActs[i]->setVisible(  true     );
    }
    for( int j = numRecentItems; j < _maxRecentItems; ++j )
    {
        Q_ASSERT( _recentActs[j] );
        _recentActs[j]->setVisible( false );
    }

    Q_ASSERT( _separatorAct );
    _separatorAct->setVisible( numRecentItems > 0 );
}


const QString RecentFiles::getCurrentName()
{
    if( _names.size() == 0 )
        return QString();

    return _names[0];
}


void RecentFiles::setCurrentName( const QString &name )
{
    _changed = true;

    setWindowFilePath( name );

    _names.removeAll( name );
    _names.prepend(   name );
    while( _names.size() > (int)_maxRecentItems )
        _names.removeLast();

    _updateRecentActions();
    _updateWindowTitle();
}


void RecentFiles::setWindowTitleBase( const QString& titleBase )
{
    _titleBase = titleBase;
    _updateWindowTitle();
}


void RecentFiles::_updateWindowTitle()
{
    if( !_adjustTitle )
        return;

    QString title = _titleBase;

    const QString currentName = getCurrentName();
    if( currentName.size() != 0 )
        title += QString(" - ") + strippedName( currentName );

    setWindowTitle( title );
}


} //namespace GUI





