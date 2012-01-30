#!/usr/bin/python

import psutil
import time
import datetime
from optparse import OptionParser

from common import *

seekTimeSec = 30

def getProcessRunTime( processName ):
   
   pidList = psutil.get_pid_list()
   timeSec = -1
   for pid in pidList:
      try:
         process = psutil.Process( pid )
         if process.exe  == processName:
            timeDiffObj = datetime.datetime.now() - datetime.datetime.fromtimestamp(process.create_time)
            timeSec = timeDiffObj.seconds
            break
      except:
         x = 3
   return timeSec
   
         
if __name__ == "__main__":

   parser = OptionParser()
   parser.add_option("-a", "--application", dest="application",
                     help="Select app ( eqPly, rtneuron )", default="eqPly")
   parser.add_option("-s", "--step", dest="step",
                     help="Time steps to look", default = seekTimeSec, type="int")
   
   (options, args) = parser.parse_args()
   
   binaryPath = ''
      
   if options.application == 'eqPly':
      binaryPath = eqPlyBinaryPath
   elif options.application == 'rtneuron':
      binaryPath = rtNeuromBinaryPath
   else:
      exit()

   while( True ):
      processTime = getProcessRunTime( binaryPath )
      print "CPU Time of " + binaryPath + " is: " + str( processTime )
      if processTime > timeSecToWaitForProcess:
         print "Possible hang"
         if options.application == 'eqPly':
            os.system('killall -9 eqPly')
            os.system('cexec killall -9 eqPly')
         elif options.application == 'rtneuron':
            os.system('killall -9 rtneuron.equalizer')
            os.system('cexec killall -9 rtneuron.equalizer')

      time.sleep( seekTimeSec )


   
