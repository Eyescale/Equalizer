/* Copyright (c) 2006, Dustin Wueest <wueest@dustin.ch> 
   All rights reserved. */

#include "tracker.h"
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/termios.h>
#include <sys/select.h>

#define COMMAND_POS_ANG "Y"
#define COMMAND_POINT "B"

using namespace std;

Tracker::Tracker()
         : _running( false )
{
    _scale[0] = -1.0/18000;
    _scale[1] = -1.0/18000;
    _scale[2] = 1.0/18000;
}

bool Tracker::init( const string& port )
{
   if( _running )
   {
      cout << "Duplicate tracker initialisation" << endl;
      return false;
   }
 
   _fd = open( port.c_str(), O_RDWR | O_EXCL );
   if( _fd < 0 )
   {
      cout << "Failed to open " << port << ": " << strerror( errno ) << endl;
      return false;
   }

   // configure serial port
   struct termios termio;
   if( tcgetattr( _fd, &termio ) != 0 )
   {
      cout << "tcgetattr failed: " << strerror( errno ) << endl;
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

   cfsetspeed( &termio, B9600 );
   termio.c_cc[VMIN]  = 26;
   termio.c_cc[VTIME] = 1;

   if( tcsetattr( _fd, TCSANOW, &termio ) != 0)
   {
      cout << "tcsetattr failed: " << strerror( errno ) << endl;
      close( _fd );
      return false;
   }

   //tell the tracker what kind of data to prepare
   int k = write( _fd, COMMAND_POS_ANG, 1 ); //take data
   if( k==-1 )
      cerr << "Write error: " << strerror( errno ) << endl;

   usleep( 10000 ); //give enough time for initialising
   bool b = _update(); //try an update to see if it works
   if(b)
     _running = true;
   return b;
}

bool Tracker::update()
{
   if( !_running )
   {
      cerr << "Update error, tracker not running" << endl;
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
   unsigned char buffer[12];
   int k = write( _fd, COMMAND_POINT, 1 ); //send data
   if( k==-1 )
      cerr << "Write error: " << strerror( errno ) << endl;

   bool b = _read( buffer, 12, 500000 );
   if( !b )
      cerr << "Read error: " << strerror( errno ) << endl;

   const short xpos = (buffer[1]<<8 | buffer[0]);
   const short ypos = (buffer[3]<<8 | buffer[2]);
   const short zpos = (buffer[5]<<8 | buffer[4]);

   const short head = (buffer[7]<<8 | buffer[6]);
   const short pitch = (buffer[9]<<8 | buffer[8]);
   const short roll = (buffer[11]<<8 | buffer[10]);

   const float hpr[3] = { head / 5194.81734f,    //32640 -> 2 * pi = /5194.81734
                          pitch / 5194.81734f,
                          roll / 5194.81734f };

   float pos[3];

   //highest value for y and z position of the tracker sensor is 32639,
   //after that it switches back to zero (and vice versa if descending values).
   pos[0] = ypos;
   if( pos[0] > 16320 )             //32640 / 2 = 16320
      pos[0] -= 32640;

   pos[1] = zpos;
   if( pos[1] > 16320 )
      pos[1] -= 32640;

   pos[2] = xpos;
   
   _matrix = eq::Matrix4f::IDENTITY;
   _matrix.setTranslation( pos[0], pos[1], pos[2] );
   _matrix.scaleTranslation( _scale );
   _matrix.rotateX( hpr[0] );
   _matrix.rotateY( hpr[1] );
   _matrix.rotateZ( hpr[2] );

   return b;
}

bool Tracker::_read( unsigned char* buffer, const size_t size,
                                            const unsigned long int timeout )
{
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
         cerr << "Error: no data from tracker" << endl;
         return false;
      }
      if( errCode == -1 )
      {
         cerr << "Select error: " << strerror( errno ) << endl;
         return false;
      }

      //try_to read remaining bytes, returns # of readed bytes
      const ssize_t receiver = read( _fd, &buffer[size-remaining], remaining );
      if( receiver == -1 )
      {	
         cerr << "Read error: " << strerror( errno ) << endl;
         return false;
      }

      EQASSERT( remaining >= receiver );
      remaining -= receiver;
   }
   return true;
}

const eq::Matrix4f& Tracker::getHeadMatrix() const
{
    return _matrix;
}
