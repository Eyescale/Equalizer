#!/usr/bin/python

import psutil
import time

from common import *

seekTimeSec = 30

def getProcessRunTime( processName ):
   
   pidList = psutil.get_pid_list()
   timeSec = -1
   for pid in pidList:
      try:
         process = psutil.Process( pid )
         if process.exe  == processName:
            timeSec = process.get_cpu_times().user
            break
      except:
         x = 3
   return timeSec
   
         
if __name__ == "__main__":

   while( True ):
      processTime = getProcessRunTime( eqPlyBinaryPath )
      print "CPU Time of " + eqPlyBinaryPath + " is: " + str( processTime )
      if processTime > timeSecToWaitForProcess:
         print "Possible hang"
         os.system('killall -9 eqPly')
         os.system('cexec killall -9 eqPly')
      time.sleep( seekTimeSec )


   
