#!/usr/bin/python

import startServers
from common import *
import os
import glob
import re
from optparse import OptionParser

dirNameGPUCountDict = dict()

def getGPUCountFromLogfile( nodeLogFile ):
   
   logFile = open( nodeLogFile,"r")
   fileStr = logFile.read()
   logFile.close()

   numberOfGPUs = len([(a.start(), a.end()) for a in list(re.finditer("GLXEW", fileStr))])
   return numberOfGPUs

def fixTestsForConfigEqPly( config ):
   
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
      
      
def fixTestsForConfigRTNeuron( config ):
   
   if not os.path.exists( config.dirName ):
      return
   
   subDirName = config.dirName + "/" + str( config.serverCount )
   if not os.path.exists( subDirName ):
      return

   files = glob.glob( subDirName + "/node*.log" )

   nodeDict = dict()

   for serverCount in startServers.getActiveServers( range( 1, config.serverCount + 1 ) ):
      searchText = "node" + str(serverCount).zfill(2)
      for fileName in files:
         if ( fileName.find( searchText ) > 0 ):
            if( not nodeDict.has_key( searchText ) ):
               nodeDict[ searchText ] = 1
            else:
               nodeDict[ searchText ] = nodeDict[ searchText ] + 1
   
   nodeCount = len( nodeDict.keys() )  
   
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
   for fileName in files:
      dirNameGPUCountDict[ newSubDirName ] = dirNameGPUCountDict[ newSubDirName ] + getGPUCountFromLogfile( fileName )
    
def fixTests( schema, application ):

   fixFunction = fixTestsForConfigEqPly
   if application == 'eqPly':
      fixFunction = fixTestsForConfigEqPly
   elif application == 'rtneuron':
      fixFunction = fixTestsForConfigRTNeuron
   else:
      exit()
      
   for serverCount in range( 1, numberOfServers + 1 ):
      testScheme( schema, application, fixFunction, serverCount)
     
   for fileDir in dirNameGPUCountDict.keys():
      print fileDir + " " + str( dirNameGPUCountDict[ fileDir ] )
      gpuCountFilePtr = open( fileDir + "/" + gpuCountFile,"w")
      gpuCountFilePtr.write( str( dirNameGPUCountDict[ fileDir ] ) )
      gpuCountFilePtr.close()

if __name__ == "__main__":
   
   parser = OptionParser()
   parser.add_option("-a", "--application", dest="application",
                     help="Select app ( eqPly, rtneuron )", default="eqPly")
   parser.add_option("-m", "--schema", dest="schema",
                     help="Schema to test ( single, combination )", default = "combination")
                     
   (options, args) = parser.parse_args()
   
   fixTests( options.schema, options.application )

