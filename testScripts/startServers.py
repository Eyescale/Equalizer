#!/usr/bin/python

import os
import sys
import getopt
import subprocess
import time

numberOfServers = 13
excludedServers = [] 

# Killall the servers in range
def stopServersInRange( serverRange ):
   os.system("killall -9 gpu_sd")
   os.system("cexec killall -9 gpu_sd")

# Start the servers in range 
def startServersInRange( serverRange, sessionName ):
 
   for i in serverRange:
      if i in excludedServers:
         continue

      nodeNumberStr = str(i).zfill(2)
      cmdStr = "ssh bilgili@node%s gpu_sd -s %s " % ( nodeNumberStr, sessionName )
      print cmdStr
      subprocess.Popen( [ cmdStr ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE, shell=True)

def startServers( firstServer, lastServer, sessionName ):
   stopServersInRange( range( 1, numberOfServers + 1 ) ) # Killall servers
   startServersInRange( range( firstServer, lastServer + 1 ), sessionName )
   time.sleep(30)


def main():
   if len( sys.argv ) < 2:
      print "Start server and Stop server should be provided"
      exit()
      
   for arg in sys.argv:
      process(arg)

if __name__ == "__main__":
    main()



