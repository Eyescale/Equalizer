#!/usr/bin/python

import os
import numpy
import copy

from common import *
from optparse import OptionParser

import fixTests

dirNameGPUCountLFPSArrayDict = dict()
dirNameGPUCountMFPSArrayDict = dict()
dirNameGPUCountHFPSArrayDict = dict()

def convertToDictEqPly( config ):

   if not os.path.exists( config.dirName ):
      return
      
   subDirName = config.dirName + "/" + str( config.serverCount )   
   if not os.path.exists( subDirName ):
      return    
  
   testfName = subDirName + "/" + testFileName

   if not os.path.exists( testfName ):
      return
      
   if not dirNameGPUCountMFPSArrayDict.has_key( config.dirName ):
      dirNameGPUCountMFPSArrayDict[ config.dirName ] = dict()

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
   
   dirNameGPUCountMFPSArrayDict[ config.dirName ][ gpuCountData ] = float( maxFrameRate )
   
def convertToDictRTNeuron( config ):

   if not os.path.exists( config.dirName ):
      return
      
   subDirName = config.dirName + "/" + str( config.serverCount )   
   if not os.path.exists( subDirName ):
      return    
  
   testfName = subDirName + "/" + rtneuronFPSFile

   if not os.path.exists( testfName ):
      return
      
   if not dirNameGPUCountMFPSArrayDict.has_key( config.dirName ):
      dirNameGPUCountLFPSArrayDict[ config.dirName ] = dict()
      dirNameGPUCountMFPSArrayDict[ config.dirName ] = dict()
      dirNameGPUCountHFPSArrayDict[ config.dirName ] = dict()
    
   fpsData = numpy.genfromtxt( testfName, dtype=None )
   fpsData = fpsData[20:-5]
      
   # Find standard deviation
   sd = numpy.std( fpsData )
   
   # Find mean
   mn = numpy.mean( fpsData )
   
   # how far is it from the mean
   howFar = 100
   lLimit = mn - howFar * sd   
   hLimit = mn + howFar * sd

   print 1000 / mn 
   
   filteredFPSData = []
         
   for n in fpsData:
      if( n >= lLimit and n <= hLimit):
         filteredFPSData.append( 1000 / n ) # per frame time is converted to fps
         
   sfFPSData = numpy.sort( filteredFPSData )
         
   print len( sfFPSData )
   
   minFrameRate = numpy.mean( sfFPSData[0:10] )
   meanFrameRate = numpy.mean( sfFPSData )
   maxFrameRate = numpy.mean( sfFPSData[-10:] )
   
   gpufName = subDirName + "/" + gpuCountFile
   
   gpuCountData = int( numpy.genfromtxt( gpufName, dtype=None ) )
   
   dirNameGPUCountLFPSArrayDict[ config.dirName ][ gpuCountData ] = float( minFrameRate )   
   dirNameGPUCountMFPSArrayDict[ config.dirName ][ gpuCountData ] = float( meanFrameRate )   
   dirNameGPUCountHFPSArrayDict[ config.dirName ][ gpuCountData ] = float( maxFrameRate )   
  
def writeDictToFile( dictObj, filename ):

   for dirName in dictObj.keys():
      print dirName
      gpuCountList = copy.deepcopy( dictObj[ dirName ].keys() )
      gpuCountList.sort()
      f = open( dirName + "/" + filename, "w" )
      for gpuCount in gpuCountList:
         f.write( str(gpuCount) + " " + str(dictObj[ dirName ][ gpuCount ]) + "\n" )
      f.close()

  
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

   writeDictToFile( dirNameGPUCountLFPSArrayDict, gpuCountLFPSFile )
   writeDictToFile( dirNameGPUCountMFPSArrayDict, gpuCountFPSFile )
   writeDictToFile( dirNameGPUCountHFPSArrayDict, gpuCountHFPSFile )


  
if __name__ == "__main__":
   
   parser = OptionParser()

   parser.add_option("-a", "--application", dest="application",
                     help="Select app ( eqPly, rtneuron )", default="eqPly")
   parser.add_option("-m", "--schema", dest="schema",
                     help="Schema to test ( single, combination )", default = "combination")

   (options, args) = parser.parse_args()
   
   convert( options.schema, options.application )

