
/* Copyright (c) 2011, Maxim Makhinya <maxmah@gmail.com>
 *
 */


#ifndef MASS_VOL__GUI_SETTINGS_H
#define MASS_VOL__GUI_SETTINGS_H

#include <QtCore/QCoreApplication>

#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif

namespace CoreSettings
{

// settings for QSettings initialization, company and product names
#define COMPANY_NAME "Makhinya"
#define PRODUCT_NAME "volVisGUI"

VARIABLE_IS_NOT_USED static void setSettingsNames()
{
    QCoreApplication::setOrganizationName( COMPANY_NAME );
    QCoreApplication::setApplicationName(  COMPANY_NAME );
}

}// namespace CoreSettings


#endif // MASS_VOL__GUI_SETTINGS_H
