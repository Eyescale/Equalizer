#!/usr/bin/python

import os
import numpy
import copy

from common import *
from optparse import OptionParser

import fixTests

dirNameGPUCountFPSArrayDict = dict()

def convertToDictEqPly( config ):

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

   testf = open( testfName, "r" )
   fpsData = []
   for line in testf.readlines():
      lst = line.split(",")
      fpsData.append( float( lst[1] ) )
   testf.close()
    
   maxFrameRate = max( fpsData )
   
   gpufName = subDirName + "/" + gpuCountFile
   
   gpuf = open( gpufName, "r" )
   
   gpuCountData = int( numpy.genfromtxt( gpufName, dtype=None ) )
   
   dirNameGPUCountFPSArrayDict[ config.dirName ][ gpuCountData ] = float( maxFrameRate )
   
def convertToDictRTNeuron( config ):

   if not os.path.exists( config.dirName ):
      return
      
   subDirName = config.dirName + "/" + str( config.serverCount )   
   if not os.path.exists( subDirName ):
      return    
  
   testfName = subDirName + "/" + rtneuronFPSFile

   if not os.path.exists( testfName ):
      return
      
   if not dirNameGPUCountFPSArrayDict.has_key( config.dirName ):
      dirNameGPUCountFPSArrayDict[ config.dirName ] = dict()
    
   fpsData = numpy.genfromtxt( testfName, dtype=None )
   fpsData = fpsData[20:-5]
      
   # Find standard deviation
   sd = numpy.std( fpsData )
   
   # Find mean
   mn = numpy.mean( fpsData )
   
   # how far is it from the mean
   howFar = 1 
   lLimit = mn - howFar * sd   
   hLimit = mn + howFar * sd

   print 1000 / mn 
   
   filteredFPSData = []
         
   for n in fpsData:
      if( n >= lLimit and n <= hLimit):
         filteredFPSData.append( 1000 / n ) # per frame time is converted to fps
         
   print len( filteredFPSData )
   
   maxFrameRate = numpy.mean( filteredFPSData )
   
   gpufName = subDirName + "/" + gpuCountFile
   
   gpuCountData = int( numpy.genfromtxt( gpufName, dtype=None ) )
   
   dirNameGPUCountFPSArrayDict[ config.dirName ][ gpuCountData ] = float( maxFrameRate )   
   
  
def convert( schema, application ):

   fixTests.fixTests( schema, application )
   
   convertToDict = convertToDictEqPly
   if application == 'eqPly':
      convertToDict = convertToDictEqPly
   elif application == 'rtneuron':
      convertToDict = convertToDictRTNeuron
   else:
      exit()
   
   for serverCount in range( 1, numberOfServers + 1 ):
      testScheme( schema, application, convertToDict, serverCount )

   for dirName in dirNameGPUCountFPSArrayDict.keys():
      print dirName
      gpuCountList = copy.deepcopy( dirNameGPUCountFPSArrayDict[ dirName ].keys() )
      gpuCountList.sort()
      f = open( dirName + "/" + gpuCountFPSFile, "w" )
      for gpuCount in gpuCountList:
         f.write( str(gpuCount) + " " + str(dirNameGPUCountFPSArrayDict[ dirName ][ gpuCount ]) + "\n" )
      f.close()


if __name__ == "__main__":
   
   parser = OptionParser()

   parser.add_option("-a", "--application", dest="application",
                     help="Select app ( eqPly, rtneuron )", default="eqPly")
   parser.add_option("-m", "--schema", dest="schema",
                     help="Schema to test ( single, combination )", default = "combination")

   (options, args) = parser.parse_args()
   
   convert( options.schema, options.application )

