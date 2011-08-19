
/* Copyright (c) 2010-2011, Stefan Eilemann <eile@eyescale.ch> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef COBASE_H
#define COBASE_H

/**
 * @namespace co::base
 * @brief Base abstraction layer and common utility classes.
 *
 * The co::base namespace provides C++ classes to abstract the underlying
 * operating system and implements common helper functionality. Classes with
 * non-virtual destructors are not intended to be subclassed.
 */

#include <co/base/api.h>
#include <co/base/atomic.h>
#include <co/base/debug.h>
#include <co/base/errorRegistry.h>
#include <co/base/file.h>
#include <co/base/global.h>
#include <co/base/perThread.h>
#include <co/base/rng.h>
#include <co/base/scopedMutex.h>
#include <co/base/sleep.h>
#include <co/base/spinLock.h>

#ifdef EQ_SYSTEM_INCLUDES
#  include <co/base/os.h>
#endif

#endif // COBASE_H

