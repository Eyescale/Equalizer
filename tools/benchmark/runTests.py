#!/usr/bin/python

import startServers
import sys 
from optparse import OptionParser
import numpy
import pickle
import subprocess

from common import *
import os

forceRedo = False
checkAndRedo = False
multiprocess = False
target = ''
sgraph = ''
cull = ''



def emtpyDirectory( dirName ):
   print "Empty directory : " + dirName
   os.system( 'rm %s/*.log' % ( dirName ) )

def checkEqPlyTestDirectory( dirName ):
   
   if not os.path.exists( dirName ):
      return  False
      
   testfName = dirName + "/" + testFileName

   if not os.path.exists( testfName ):
      return False
      
   fpsData = numpy.genfromtxt( testfName, dtype=float )
   
   if( fpsData.size < 4 ):
      return False
      
   return True

def checkRTNeuronTestDirectory( dirName ):
   
   if not os.path.exists( dirName ):
      return  False
      
   testfName = dirName + "/" + rtneuronFPSFile

   if not os.path.exists( testfName ):
      return False
      
   fpsData = numpy.genfromtxt( testfName, dtype=float )
   
   if( fpsData.size < 100 ):
      return False
      
   return True
      

def testEqPly( config ):
   
   if not os.path.exists( config.dirName ):
      os.mkdir( config.dirName )
   
   saveCurrentDir()

   subDirName = config.dirName + "/" + str( config.serverCount )   
   
   if not os.path.exists( subDirName ):
      os.mkdir( subDirName )
   
   if( forceRedo ):
      emtpyDirectory( subDirName )
   
   if( checkAndRedo ):
      if( checkEqPlyTestDirectory( subDirName ) ):
         gotoPreviousDir()
         return
      else:
         emtpyDirectory( subDirName )
      
   os.chdir( subDirName )
   
   roiStr = ''   
   if( config.roiState == 'ROIDisabled' ):
      roiStr = ' -d '

   eqLayoutArg = '--eq-layout "%s" ' % ( config.layoutName )
   eqPlyConfigArg = '--eq-config "%s" ' % ( config.session )
   nbOfFramesArg = '-n ' + str(config.nbOfFrames) 
   multiProcessStr = ''
   if( multiprocess ):
      multiProcessStr = '-f'

   os.system('killall -9 eqPly')
   os.system('cexec killall -9 eqPly')

   startServers.startServers( 1, config.serverCount, config.session )
   cmdStr = eqPlyBinaryPath + ' ' + eqPlyConfigArg + ' ' + eqPlyDefaultArgs + ' ' + roiStr + ' ' + eqLayoutArg + ' ' + nbOfFramesArg + ' ' + multiProcessStr 

   print cmdStr
   
   writeCommandStringToFile( cmdStr )

   os.system( cmdStr )

   gotoPreviousDir()
   

def testRTNeuron( config ):

   if not os.path.exists( config.dirName ):
      os.mkdir( config.dirName )
   
   saveCurrentDir()

   subDirName = config.dirName + "/" + str( config.serverCount )   
   if not os.path.exists( subDirName ):
      os.mkdir( subDirName )
   
   if( forceRedo ):
      emtpyDirectory( subDirName )
   
   if( checkAndRedo ):
      if( checkRTNeuronTestDirectory( subDirName ) ):
         gotoPreviousDir()
         return
      else:
         emtpyDirectory( subDirName )
         
         
   os.chdir( subDirName )
   roiStr = ''   
   rtLayoutArg = ''
   
   if( config.layoutName == "Static2D" or config.layoutName == "Dynamic2D" ):
      rtLayoutArg = '--eq-layout "%s" ' % ( config.layoutName )
   elif( config.layoutName == "RoundRobinDB" ):
      rtLayoutArg = '--eq-layout StaticDB --round-robin-DB-partition'
   elif( config.layoutName == "SpatialDB" ):
      rtLayoutArg = '--eq-layout StaticDB --spatial-DB-partition'
   elif( config.layoutName == "DBDirectSendSDB" ):
      rtLayoutArg = '--eq-layout DBDirectSend --spatial-DB-partition'
   elif( config.layoutName == "DBDirectSendRR" ):
      rtLayoutArg = '--eq-layout DBDirectSend --round-robin-DB-partition'
      
   rtNeuronConfigArg = '--eq-config "%s" ' % ( config.session )
  
   nbOfFramesArg = '--frame-count ' + str(config.nbOfFrames) 
   
   roiStr = ''   
   if( config.roiState == 'ROIEnabled' ):
      roiStr = ' --roi '
      
   os.system('killall -9 rtneuron.equalizer')   
   os.system('cexec killall -9 rtneuron.equalizer')
   
   startServers.startServers( 1, config.serverCount, config.session )

   cmdStr = rtNeuromBinaryPath + ' ' + rtNeuronConfigArg + ' ' + rtNeuronDefaultArgs + ' ' + roiStr + ' ' + rtLayoutArg + ' ' + nbOfFramesArg + ' ' + target + ' ' + sgraph + ' ' + cull

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
   parser.add_option("-r", "--forceRedo", dest="forceRedo",
                     action="store_true", help="Force Redo the test", default = False )
   parser.add_option("-c", "--checkAndRedo", dest="checkAndRedo",
                     action="store_true", help="Check and Redo the test", default = False )
   parser.add_option("-u", "--multiprocess", dest="multiprocess",
                     action="store_true", help="Force multi process", default = False )
   parser.add_option("-j", "--justlist", dest="justlist",
                     action="store_true", help="Just list, do not run", default = False )
   parser.add_option("-y", "--async", dest="async",
                     action="store_true", help="Enable asyncronous thread model", default = False )
   parser.add_option("-t", "--target", dest="target",
                     help="Target to render ( col, mcolx (x = 0:50))", default = "col")
   parser.add_option("-o", "--octree", dest="octree",
                     help="rtneuron only, use octree", default = -1, type="int" )
   parser.add_option("-g", "--gpu", dest="gpu",
                      action="store_true", help="Disable gpu cull", default=False )
   parser.add_option("-e", "--readtestoption", dest="readtestoption",
                      action="store_true", help="Read previous test option", default=False )

   (options, args) = parser.parse_args()

   if( options.readtestoption ):
    if( os.path.exists( optionsDumpFilename ) ):
      with open( optionsDumpFilename, "r" ) as fp:
         options = pickle.load(fp)
         options.readtestoption = False
    else:
      print "Options cannot be read"
      exit()
   
   with open( optionsDumpFilename, "w" ) as fp:
      pickle.dump(options, fp)

   global checkAndRedo
   global forceRedo
   global multiprocess
   global target
   global sgraph
   global cull
   
   if( options.octree >= 0):
      sgraph = "--use-octree --octree-depth " + str( options.octree )
   
   if( options.gpu ):
      cull = "--no-cuda"
   
   forceRedo = options.forceRedo
   checkAndRedo = options.checkAndRedo
   multiprocess = options.multiprocess
   
   if( options.target == "col" ):
      target = '--target mc0_Column mesh none 1 0 0 1.0'
   elif( options.target[0:4] == "mcol" ):
      count = int( options.target[4:] )
      for i in range(0, int(count)):
			target +=  " --target MiniColumn_" + str(i) + " mesh none 1 0 0 1.0 "
   
   if( options.async ):
      os.environ['EQ_NODE_IATTR_THREAD_MODEL'] = str(-9)
   
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
      print startServers.getActiveServers( range(1, serverCount+1) )
      if( not options.justlist ):
         testScheme( options.schema, options.application, testFunc, serverCount )


if __name__ == "__main__":
    main()

