#!/usr/bin/python

import startServers
import sys 
from optparse import OptionParser

from common import *
import os

def testEqPly( config ):
   
   if not os.path.exists( config.dirName ):
      os.mkdir( config.dirName )
   
   # os.system('ssh node01 killall -9 eqPly')
   os.system('killall -9 eqPly')
   os.system('cexec killall -9 eqPly')

   saveCurrentDir()

   subDirName = config.dirName + "/" + str( config.serverCount )   
   if not os.path.exists( subDirName ):
      os.mkdir( subDirName )
      
   os.chdir( subDirName )
   
   roiStr = ''   
   if( config.roiState == 'ROIDisabled' ):
      roiStr = ' -d '

   eqLayoutArg = '--eq-layout "%s" ' % ( config.layoutName )
   eqPlyConfigArg = '--eq-config "%s" ' % ( config.session )
   nbOfFramesArg = '-n ' + str(config.nbOfFrames) 

   startServers.startServers( 1, config.serverCount, config.session )
   cmdStr = eqPlyBinaryPath + ' ' + eqPlyConfigArg + ' ' + eqPlyDefaultArgs + ' ' + roiStr + ' ' + eqLayoutArg + ' ' + nbOfFramesArg

   print cmdStr
   
   writeCommandStringToFile( cmdStr )

   os.system( cmdStr )

   gotoPreviousDir()
   

def testRTNeuron( config ):
   
   if not os.path.exists( config.dirName ):
      os.mkdir( config.dirName )
   
   # os.system('ssh node01 killall -9 eqPly')
   os.system('killall -9 rtneuron.equalizer')
   os.system('cexec killall -9 rtneuron.equalizer')

   saveCurrentDir()

   subDirName = config.dirName + "/" + str( config.serverCount )   
   if not os.path.exists( subDirName ):
      os.mkdir( subDirName )
      
   os.chdir( subDirName )
   roiStr = ''   
   rtLayoutArg = ''
   
   if( config.layoutName == "Static2D" or config.layoutName == "Dynamic2D" ):
      rtLayoutArg = '--eq-layout "%s" ' % ( config.layoutName )
   elif( config.layoutName == "RoundRobinDB" ):
      rtLayoutArg = '--eq-layout StaticDB --round-robin-DB-partition'
   elif( config.layoutName == "SpatialDB" ):
      rtLayoutArg = '--eq-layout StaticDB --spatial-DB-partition'
      
   rtNeuronConfigArg = '--eq-config "%s" ' % ( config.session )
  
   nbOfFramesArg = '--frame-count ' + str(config.nbOfFrames) 
   
   roiStr = ''   
   if( config.roiState == 'ROIEnabled' ):
      roiStr = ' --roi '
   
   startServers.startServers( 1, config.serverCount, config.session )
   cmdStr = rtNeuromBinaryPath + ' ' + rtNeuronConfigArg + ' ' + rtNeuronDefaultArgs + ' ' + roiStr + ' ' + rtLayoutArg + ' ' + nbOfFramesArg + ' ' + roiStr

   print cmdStr
   
   writeCommandStringToFile( cmdStr )

   os.system( cmdStr )
 
   gotoPreviousDir()   
   
   
def setFulscreenMode( mode ):

   os.environ['EQ_WINDOW_IATTR_HINT_FULLSCREEN'] = str( mode )


def main():

   parser = OptionParser()
   parser.add_option("-f", "--fullscreen",
                     action="store_true", dest="screenmode", default=False )
   parser.add_option("-a", "--application", dest="application",
                     help="Select app ( eqPly, rtneuron )", default="eqPly")
   parser.add_option("-s", "--servercount", dest="serverCount",
                     help="Number of servers to be tested", default = 13, type="int")
   parser.add_option("-p", "--step", dest="step",
                     help="Servers in range startServer to endServer tested in steps", default = 1, type="int")
   parser.add_option("-b", "--beginServer", dest="beginServer",
                     help="Servers in range beginServer to beginServer + number of servers will be  tested in steps", default = 1, type="int")
   parser.add_option("-m", "--schema", dest="schema",
                     help="Schema to test ( single, combination )", default = "combination")

   (options, args) = parser.parse_args()
  
   setFulscreenMode( options.screenmode )
   
   testFunc = testEqPly
   if options.application == 'eqPly':
      testFunc = testEqPly
   elif options.application == 'rtneuron':
      testFunc = testRTNeuron
   else:
      print "No proper application selected"
      exit()

   maxServer = options.beginServer + options.serverCount - 1
   if( maxServer > numberOfServers ):
      maxServer = numberOfServers
      
   for serverCount in range( options.beginServer,  maxServer + 1,  options.step ):
      # print "Server count: ", serverCount
      testScheme( options.schema, options.application, testFunc, serverCount )


if __name__ == "__main__":
    main()

