#!/usr/bin/python

import startServers
from common import *
import os
import glob
import re

dirNameGPUCountDict = dict()

def getGPUCountFromLogfile( nodeLogFile ):
   
   logFile = open( nodeLogFile,"r")
   fileStr = logFile.read()
   logFile.close()

   numberOfGPUs = len([(a.start(), a.end()) for a in list(re.finditer("GLXEW", fileStr))])
   return numberOfGPUs

def fixTestsForConfig( config ):
   
   # if( config.serverCount == 1 ):
   #   return

   if not os.path.exists( config.dirName ):
      return
   
   subDirName = config.dirName + "/" + str( config.serverCount )
   if not os.path.exists( subDirName ):
      return

   files = glob.glob( subDirName + "/node*.log" )

   nodeCount = len( files )  
   
   newSubDirName = subDirName
   
   if ( nodeCount != config.serverCount ):
      newSubDirName =  config.dirName + "/" + str( nodeCount )
      if os.path.exists( newSubDirName ):
         print( "Tests already exists for node count: " + str( nodeCount ) )
      else:
         os.rename( subDirName,  newSubDirName )
   
   if not dirNameGPUCountDict.has_key( newSubDirName ):
      dirNameGPUCountDict[ newSubDirName ] = 0 
  
   files = glob.glob( newSubDirName + "/node*.log" )
   
   for logFile in files:
      gpuCount = getGPUCountFromLogfile( logFile )
      dirNameGPUCountDict[ newSubDirName ] = dirNameGPUCountDict[ newSubDirName ] + gpuCount
    
def fixTests():
   for serverCount in range( 1, numberOfServers + 1 ):
      testScheme( "eqPly", fixTestsForConfig, serverCount)
     
   for fileDir in dirNameGPUCountDict.keys():
      print fileDir + " " + str( dirNameGPUCountDict[ fileDir ] )
      gpuCountFilePtr = open( fileDir + "/" + gpuCountFile,"w")
      gpuCountFilePtr.write( str( dirNameGPUCountDict[ fileDir ] ) )
      gpuCountFilePtr.close()
    
def main():
   fixTests()   

if __name__ == "__main__":
    main()
