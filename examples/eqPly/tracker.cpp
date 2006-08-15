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
{}

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

   termio.c_cflag &= ~(CSIZE|PARENB|PARODD|HUPCL); 
   termio.c_cflag |= CS8|CSTOPB|CREAD|CLOCAL|CRTSCTS;
   termio.c_iflag &= ~(IXON|IUCLC|IXANY|IMAXBEL|BRKINT|IGNPAR|PARMRK|
                       INPCK|ISTRIP|INLCR|IGNCR|ICRNL);
   termio.c_iflag |= IXOFF|IGNBRK;
   termio.c_oflag &= ~(OPOST|OLCUC|OCRNL|ONLCR|ONOCR|ONLRET|OFILL|OFDEL);
   termio.c_lflag &= ~(ISIG|ICANON|IEXTEN|ECHO|ECHOE|ECHOK|ECHONL|NOFLSH|
                       XCASE|TOSTOP|ECHOPRT|ECHOCTL|ECHOKE);

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

   short xpos = (buffer[1]<<8 | buffer[0]);
   short ypos = (buffer[3]<<8 | buffer[2]);
   short zpos = (buffer[5]<<8 | buffer[4]);

   short head = (buffer[7]<<8 | buffer[6]);
   short pitch = (buffer[9]<<8 | buffer[8]);
   short roll = (buffer[11]<<8 | buffer[10]);

   //TODO: make following part generic, with rotation matrices...
   float pos[3];
   float hpr[3];
   pos[0] = ypos / 18000.0;
   if( pos[0] > 0.906639 )          //32639 / 18000 = 1.813278
      pos[0] -= 1.813278;           //1.813278 / 2 = 0.906639

   pos[1] =  zpos / 18000.0;
   if( pos[1] > 0.906639 )
      pos[1] -= 1.813278;

   pos[2] = -xpos / 18000.0;        //measured on 29.7cm with DinA4

   hpr[0] = head / 90.6639;
   hpr[1] = pitch / 90.6639;
   hpr[2] = roll / 90.6639;         //32639 -> 360 degree = /90.66389

   _matrix.setTranslation( pos[0], pos[1], pos[2] );
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
   int k;

   tv.tv_sec = timeout / 1000000;
   tv.tv_usec = timeout % 1000000;

   fd_set readfds;
   while( remaining > 0 )
   {
      // wait for data
      fd_set readfds;
      FD_ZERO( &readfds );
      FD_SET( _fd, &readfds );

      k = select( _fd+1, &readfds, NULL, NULL, &tv );
      if( k== 0 )
      {
         cerr << "Error: no data from tracker" << endl;
         return false;
      }
      if( k==-1 )
      {
         cerr << "Select error: " << strerror( errno ) << endl;
         return false;
      }

      //try_to read remaining bytes, returns # of readed bytes
      k = read( _fd, &buffer[size-remaining], remaining );
      if( k==-1 )
      {	
         cerr << "Read error: " << strerror( errno ) << endl;
         return false;
      }

      remaining -= k;
      if(remaining == 0) //size_t is unsigned, can't be <0
      {
         return true;
      }
   }
}

const eq::Matrix4f& Tracker::getHeadMatrix()
{
    return _matrix;
}
