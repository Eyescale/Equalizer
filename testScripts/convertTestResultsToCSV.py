#!/usr/bin/python

import startServers
import os
import numpy
import runTests

testFileName = "FPSInfo.txt"
resultsDir = "Results"

def convertToCSV( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):

   if not os.path.exists( resultsDir ):
      os.mkdir( resultsDir )
   
   if not os.path.exists( dirName ):
      print "Error finding the test directory: " + dirName
      exit()
      
   nodeFPSArray = []
   
   for serverCount in range( 1, startServers.numberOfServers + 1 ):
      
      oldDir = os.getcwd()
      
      subDirName = dirName + "/" + str( serverCount )   
      if not os.path.exists( subDirName ):
          print "Error finding the test directory: " + subDirName
          exit()
         
      os.chdir( subDirName )
 
      fpsData = numpy.genfromtxt( testFileName, dtype=None ) 
      maxFrameRate = max( fpsData )
      
      nodeFPSArray.append( maxFrameRate )
      
      os.chdir( oldDir )
      
   oldDir = os.getcwd()  
   os.chdir( resultsDir )
   
   numpy.savetxt( dirName + ".txt", nodeFPSArray, fmt="%3.2f", delimiter=',' )
   
   os.chdir( oldDir )
   
  
def main():
    runTests.testScheme( convertToCSV )

if __name__ == "__main__":
    main()

