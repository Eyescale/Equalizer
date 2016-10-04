
/* Copyright (c) 2010-2016, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQFABRIC_H
#define EQFABRIC_H

/**
 * @namespace eq::fabric
 * @brief The Equalizer data synchronization fabric.
 *
 * This namespace implements the common data management and transport layer
 * between the Equalizer applications, the server and the administrative
 * interface, which use the eq, eq::server and eq::admin namespace,
 * respectively.
 */

#include <eq/fabric/api.h>
#include <eq/fabric/axisEvent.h>
#include <eq/fabric/buttonEvent.h>
#include <eq/fabric/commands.h>
#include <eq/fabric/configParams.h>
#include <eq/fabric/configVisitor.h>
#include <eq/fabric/drawableConfig.h>
#include <eq/fabric/equalizerTypes.h>
#include <eq/fabric/errorRegistry.h>
#include <eq/fabric/iAttribute.h>
#include <eq/fabric/keyEvent.h>
#include <eq/fabric/pointerEvent.h>
#include <eq/fabric/sizeEvent.h>

#endif // EQFABRIC_H
