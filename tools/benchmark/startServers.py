#!/usr/bin/python

import os
import sys
import getopt
import subprocess
import time
import common

import checkXServersAndRestart

from common import *

# Killall the servers in range
def stopServers():
   # os.system("ssh node01 killall -9 gpu_sd")
   os.system("killall -9 gpu_sd")
   os.system("cexec killall -9 gpu_sd")

def getActiveServers( serverRange ):

   serverCount = len( serverRange )
   count = 0
   index = serverRange[0] - 1
   serverList = []
   while( count < serverCount ):
      index = index + 1
      if( index in common.excludedServers ):
         continue
      serverList.append( index )
      count = count + 1
   
   return serverList    
# Start the servers in range 
def startServersInRange( serverRange, session ):
 
   serverList = getActiveServers( serverRange )
   for i in serverList:
    
      nodeNumberStr = str(i).zfill(2)
      cmdStr = "ssh node%s gpu_sd -s %s" % ( nodeNumberStr, session )
      
      print cmdStr
      subprocess.Popen( [ cmdStr ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, shell=True)

def startServers( firstServer, lastServer, session ):
   stopServers()
   startServersInRange( range( firstServer, lastServer + 1 ), session )
   time.sleep(30)


def main():
   if len( sys.argv ) < 2:
      print "Start server and Stop server should be provided"
      exit()
      
   session = 'default'   
   if len( sys.argv ) == 4:
      session = sys.argv[3]
      
   start = int( sys.argv[1] )
   stop = int( sys.argv[2] )
   
   startServers(start, stop, session)

if __name__ == "__main__":
    main()



