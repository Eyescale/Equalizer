
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */


#include "mainWindow.h"
#include "settings.h"


#include <QtGui/QApplication>
QT_USE_NAMESPACE


int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    Q_INIT_RESOURCE( resources );

    app.setAttribute( Qt::AA_DontCreateNativeWidgetSiblings );

    CoreSettings::setSettingsNames();


/*    // This is the main controller object, which creates, links and communicates between
    // the MainWindowTmp GUI user interface and the Equalizer application.
    massVolGUI::Controller controller;
    controller.start();
*/

    massVolGUI::MainWindow window;
    window.show();

    return app.exec();
}


