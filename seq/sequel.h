
/* Copyright (c) 2011-2016, Stefan Eilemann <eile@eyescale.ch>
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

#ifndef EQ_SEQUEL_H
#define EQ_SEQUEL_H

#pragma warning(push)
#pragma warning(disable: 4244) //conversion from .. to ..,possible loss of data

/**
 * @namespace seq
 * @brief Sequel - A simple interface to the Equalizer parallel rendering
 *        framework.
 *
 * This namespace provides a simple programming interface to the Equalizer
 * parallel rendering framework. Sequel allows rapid development of clustered
 * multi-GPU applications while not sacrificing the flexibility and power of the
 * underlying Equalizer framework.
 */
#include <seq/application.h>
#include <seq/objectType.h>
#include <seq/renderer.h>
#include <seq/viewData.h>
#include <eq/eq.h>

#pragma warning(pop)
#endif // EQ_SEQUEL_H
