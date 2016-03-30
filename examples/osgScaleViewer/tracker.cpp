
/* Copyright (c) 2006-2016, Dustin Wueest <wueest@dustin.ch>
 *                          Stefan Eilemann <eile@equalizergraphics.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Eyescale Software GmbH nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "tracker.h"

#include <iostream>
#include <fcntl.h>
#include <errno.h>

#ifndef WIN32
#  include <sys/select.h>
#  include <sys/termios.h>
#endif

#define COMMAND_POS_ANG "Y"
#define COMMAND_POINT "B"

Tracker::Tracker()
        : _running( false )
        , _fd( 0 )
{
}

bool Tracker::init( const std::string& port )
{
#ifdef WIN32
    return false;
#else
   if( _running )
   {
      LBERROR << "Duplicate tracker initialisation" << std::endl;
      return false;
   }

   _fd = open( port.c_str(), O_RDWR | O_EXCL );
   if( _fd < 0 )
   {
       LBERROR << "Failed to open " << port << ": " << lunchbox::sysError
               << std::endl;
      return false;
   }

   // configure serial port
   struct termios termio;
   if( tcgetattr( _fd, &termio ) != 0 )
   {
      LBERROR << "tcgetattr failed: " << lunchbox::sysError << std::endl;
      close( _fd );
      return false;
   }

   termio.c_cflag &= ~(CSIZE|PARENB|CSTOPB|PARODD|HUPCL|CRTSCTS);
   termio.c_cflag |= CS8|CREAD|CLOCAL;
   termio.c_iflag &= ~(IXON|IXANY|IMAXBEL|BRKINT|IGNPAR|PARMRK|
                       INPCK|ISTRIP|INLCR|IGNCR|ICRNL);
#ifdef IUCLC
   termio.c_iflag &= ~IUCLC;
#endif

   termio.c_iflag |= IXOFF|IGNBRK;
   termio.c_oflag &= ~(OPOST|OCRNL|ONLCR|ONOCR|ONLRET|OFILL|OFDEL);
#ifdef OLCUC
   termio.c_oflag &= ~OLCUC;
#endif

   termio.c_lflag &= ~(ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOK|ECHONL|NOFLSH|
                       TOSTOP|ECHOPRT|ECHOCTL|ECHOKE);
#ifdef XCASE
   termio.c_lflag &= ~XCASE;
#endif

   cfsetspeed( &termio, B115200 );
   termio.c_cc[VMIN]  = 26;
   termio.c_cc[VTIME] = 1;

   if( tcsetattr( _fd, TCSANOW, &termio ) != 0)
   {
      LBERROR << "tcsetattr failed: " << lunchbox::sysError << std::endl;
      close( _fd );
      return false;
   }

   // tell the tracker what kind of data to prepare
   int k = write( _fd, COMMAND_POS_ANG, 1 ); //take data
   if( k==-1 )
      LBERROR << "Write error: " << lunchbox::sysError << std::endl;

   usleep( 10000 ); //give enough time for initializing

   if( _update( )) //try an update to see if it works
       _running = true;

   return _running;
#endif
}

bool Tracker::update()
{
   if( !_running )
   {
      LBERROR << "Update error, tracker not running" << std::endl;
      return false;
   }
   else
   {
      bool b = _update();
      return b;
   }
}

bool Tracker::_update()
{
#ifdef WIN32
    return false;
#else
    const ssize_t wrote = write( _fd, COMMAND_POINT, 1 ); // send data
   if( wrote==-1 )
   {
      LBERROR << "Write error: " << lunchbox::sysError << std::endl;
      return false;
   }

   unsigned char buffer[12];
   if( !_read( buffer, 12, 500000 ))
   {
       LBERROR << "Read error" << std::endl;
       return false;
   }

   const short xpos = (buffer[1]<<8 | buffer[0]);
   const short ypos = (buffer[3]<<8 | buffer[2]);
   const short zpos = (buffer[5]<<8 | buffer[4]);

   const short head = (buffer[7]<<8 | buffer[6]);
   const short pitch = (buffer[9]<<8 | buffer[8]);
   const short roll = (buffer[11]<<8 | buffer[10]);

   // 32640 is 360 degrees (2pi) -> scale is 1/5194.81734
   const eq::Vector3f hpr( head  / -5194.81734f + M_PI,
                             pitch / -5194.81734f + 2.0f * M_PI,
                             roll  / -5194.81734f + 2.0f * M_PI );

   eq::Vector3f pos;

   // highest value for y and z position of the tracker sensor is 32639,
   // after that it switches back to zero (and vice versa if descending values).
   pos.x() = ypos;
   if( pos.x() > 16320 )             //32640 / 2 = 16320
      pos.x() -= 32640;

   pos.y() = zpos;
   if( pos.y() > 16320 )
      pos.y() -= 32640;

   pos.z() = xpos;

   pos /= 18000.f; // scale to meter

   // position and rotation are stored in transformation matrix
   // and matrix is scaled to the application's units
   _matrix = eq::Matrix4f();
   _matrix.rotate_x( hpr.x( ));
   _matrix.rotate_y( hpr.y( ));
   _matrix.rotate_z( hpr.z( ));
   _matrix.setTranslation( pos );

   LBINFO << "Tracker pos " << pos << " hpr " << hpr << " = " << _matrix;

    // M = M_world_emitter * M_emitter_sensor * M_sensor_object
   _matrix = _worldToEmitter * _matrix * _sensorToObject;

   LBINFO << "Tracker matrix " << _matrix;

   return true;
#endif
}

bool Tracker::_read( unsigned char* buffer, const size_t size,
                     const unsigned long int timeout )
{
#ifdef WIN32
    return false;
#else
    size_t remaining = size;
   struct timeval tv;

   tv.tv_sec = timeout / 1000000;
   tv.tv_usec = timeout % 1000000;

   while( remaining > 0 )
   {
      // wait for data
      fd_set readfds;
      FD_ZERO( &readfds );
      FD_SET( _fd, &readfds );

      const int errCode = select( _fd+1, &readfds, 0, 0, &tv );
      if( errCode == 0 )
      {
         LBERROR << "Error: no data from tracker" << std::endl;
         return false;
      }
      if( errCode == -1 )
      {
         LBERROR << "Select error: " << lunchbox::sysError << std::endl;
         return false;
      }

      // try to read remaining bytes, returns # of read bytes
      const ssize_t received = read( _fd, &buffer[size-remaining], remaining );
      if( received == -1 )
      {
         LBERROR << "Read error: " << lunchbox::sysError << std::endl;
         return false;
      }

      LBASSERT( remaining >= (size_t)received );
      remaining -= received;
   }
   return true;
#endif
}
