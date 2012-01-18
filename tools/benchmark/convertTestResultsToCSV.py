#!/usr/bin/python

import os
import numpy

from common import *

def convertToCSV( config ):

   if not os.path.exists( resultsDir ):
      os.mkdir( resultsDir )
   
   if not os.path.exists( config.dirName ):
      print "Error finding the test directory: " + config.dirName
      exit()
      
   nodeFPSArray = []
   
   saveCurrentDir()
   subDirName = config.dirName + "/" + str( config.serverCount )   
   if not os.path.exists( subDirName ):
       print "Error finding the test directory: " + subDirName
       exit()
  
   os.chdir( subDirName )
   fpsData = numpy.genfromtxt( testFileName, dtype=None ) 
   maxFrameRate = max( fpsData )
   nodeFPSArray.append( maxFrameRate )
   gotoPreviousDir()
      
   saveCurrentDir()  
   os.chdir( resultsDir )
   numpy.savetxt( config.dirName + ".txt", nodeFPSArray, fmt="%3.2f", delimiter=',' )
   gotoPreviousDir()
   
  
def main():
    testScheme( "eqPly", convertToCSV )

if __name__ == "__main__":
    main()

