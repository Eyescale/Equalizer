/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   Copyright (c) 2006-2007, Stefan Eilemann <eile@equalizergraphics.com>
   All rights reserved. */

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

using namespace std;
namespace eVolve
{

Tracker::Tracker()
        : _running( false ),
          _worldToEmitter( vmml::Matrix4f::IDENTITY ),
          _sensorToObject( vmml::Matrix4f::IDENTITY )
{
}

bool Tracker::init( const string& port )
{
#ifdef WIN32
    return false;
#else
   if( _running )
   {
      EQERROR << "Duplicate tracker initialisation" << endl;
      return false;
   }
 
   _fd = open( port.c_str(), O_RDWR | O_EXCL );
   if( _fd < 0 )
   {
      EQERROR << "Failed to open " << port << ": " << strerror( errno ) << endl;
      return false;
   }

   // configure serial port
   struct termios termio;
   if( tcgetattr( _fd, &termio ) != 0 )
   {
      EQERROR << "tcgetattr failed: " << strerror( errno ) << endl;
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
      EQERROR << "tcsetattr failed: " << strerror( errno ) << endl;
      close( _fd );
      return false;
   }

   // tell the tracker what kind of data to prepare
   int k = write( _fd, COMMAND_POS_ANG, 1 ); //take data
   if( k==-1 )
      EQERROR << "Write error: " << strerror( errno ) << endl;

   usleep( 10000 ); //give enough time for initialising
   
   if( _update( )) //try an update to see if it works
       _running = true;

   return _running;
#endif
}

bool Tracker::update()
{
   if( !_running )
   {
      EQERROR << "Update error, tracker not running" << endl;
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
      EQERROR << "Write error: " << strerror( errno ) << endl;
      return false;
   }

   unsigned char buffer[12];
   if( !_read( buffer, 12, 500000 ))
   {
       EQERROR << "Read error" << endl;
       return false;
   }

   const short xpos = (buffer[1]<<8 | buffer[0]);
   const short ypos = (buffer[3]<<8 | buffer[2]);
   const short zpos = (buffer[5]<<8 | buffer[4]);

   const short head = (buffer[7]<<8 | buffer[6]);
   const short pitch = (buffer[9]<<8 | buffer[8]);
   const short roll = (buffer[11]<<8 | buffer[10]);

   // 32640 is 360 degrees (2pi) -> scale is 1/5194.81734
   const vmml::Vector3f hpr( head  / -5194.81734f + M_PI,
                             pitch / -5194.81734f + 2.0f * M_PI,
                             roll  / -5194.81734f + 2.0f * M_PI );

   vmml::Vector3f pos;

   // highest value for y and z position of the tracker sensor is 32639,
   // after that it switches back to zero (and vice versa if descending values).
   pos.x = ypos;
   if( pos.x > 16320 )             //32640 / 2 = 16320
      pos.x -= 32640;
   
   pos.y = zpos;
   if( pos.y > 16320 )
      pos.y -= 32640;

   pos.z = xpos;

   pos /= 18000.f; // scale to meter

   // position and rotation are stored in transformation matrix
   // and matrix is scaled to the application's units
   _matrix = vmml::Matrix4f::IDENTITY;
   _matrix.rotateX( hpr.x );
   _matrix.rotateY( hpr.y );
   _matrix.rotateZ( hpr.z );
   _matrix.setTranslation( pos );

   EQINFO << "Tracker pos " << pos << " hpr " << hpr << " = " << _matrix;

    // M = M_world_emitter * M_emitter_sensor * M_sensor_object
   _matrix = _worldToEmitter * _matrix * _sensorToObject;

   EQINFO << "Tracker matrix " << _matrix;

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

      const int errCode = select( _fd+1, &readfds, NULL, NULL, &tv );
      if( errCode == 0 )
      {
         EQERROR << "Error: no data from tracker" << endl;
         return false;
      }
      if( errCode == -1 )
      {
         EQERROR << "Select error: " << strerror( errno ) << endl;
         return false;
      }

      // try to read remaining bytes, returns # of read bytes
      const ssize_t received = read( _fd, &buffer[size-remaining], remaining );
      if( received == -1 )
      {
         EQERROR << "Read error: " << strerror( errno ) << endl;
         return false;
      }

      EQASSERT( remaining >= (size_t)received );
      remaining -= received;
   }
   return true;
#endif
}
}
