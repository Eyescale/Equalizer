#!/usr/bin/python

import os
import numpy
import copy

from common import *
import fixTests

dirNameGPUCountFPSArrayDict = dict()

def convertToDict( config ):

   if not os.path.exists( config.dirName ):
      return
      
   subDirName = config.dirName + "/" + str( config.serverCount )   
   if not os.path.exists( subDirName ):
      return    
  
   testfName = subDirName + "/" + testFileName

   if not os.path.exists( testfName ):
      return
      
   if not dirNameGPUCountFPSArrayDict.has_key( config.dirName ):
      dirNameGPUCountFPSArrayDict[ config.dirName ] = dict()
    
   fpsData = numpy.genfromtxt( testfName, dtype=None ) 
   maxFrameRate = max( fpsData )
   
   gpufName = subDirName + "/" + gpuCountFile
   
   gpuCountData = int( numpy.genfromtxt( gpufName, dtype=None ) )
   
   dirNameGPUCountFPSArrayDict[ config.dirName ][ gpuCountData ] = float( maxFrameRate )
  
def convert():

   fixTests.fixTests()
   
   for serverCount in range( 1, numberOfServers + 1 ):
      testScheme( "eqPly", convertToDict, serverCount )

   for dirName in dirNameGPUCountFPSArrayDict.keys():
      print dirName
      gpuCountList = copy.deepcopy( dirNameGPUCountFPSArrayDict[ dirName ].keys() )
      gpuCountList.sort()
      f = open( dirName + "/" + gpuCountFPSFile, "w" )
      for gpuCount in gpuCountList:
         f.write( str(gpuCount) + " " + str(dirNameGPUCountFPSArrayDict[ dirName ][ gpuCount ]) + "\n" )
      f.close()
   
def main():
    convert()

if __name__ == "__main__":
    main()

