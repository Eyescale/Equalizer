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
      rtLayoutArg = '--round-robin-DB-partition'
   elif( config.layoutName == "SpatialDB" ):
      rtLayoutArg = '--spatial-DB-partition'
      
   rtNeuronConfigArg = '--eq-config "%s" ' % ( config.session )
  
   nbOfFramesArg = '--frame-count ' + str(config.nbOfFrames) 

   startServers.startServers( 1, config.serverCount, config.session )
   cmdStr = rtNeuromBinaryPath + ' ' + rtNeuronConfigArg + ' ' + rtNeuronDefaultArgs + ' ' + roiStr + ' ' + rtLayoutArg + ' ' + nbOfFramesArg

   print cmdStr

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
                     help="Number of servers to be tested", default = 1)
   parser.add_option("-p", "--step", dest="step",
                     help="Servers in range startServer to endServer tested in steps", default = 1)
   
   (options, args) = parser.parse_args()
     
   setFulscreenMode( options.screenmode )
   
   testFunc = testEqPly
   if( options.application == "rtneuron" ):
      testFunc = testRTNeuron

   for serverCount in range( 1,  options.serverCount + options.step, options.step ):
      testScheme( options.application, testFunc, serverCount )


if __name__ == "__main__":
    main()

