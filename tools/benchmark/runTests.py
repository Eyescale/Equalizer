#!/usr/bin/python

import startServers
from common import *
import os

def test( config ):
   
   if not os.path.exists( config.dirName ):
      os.mkdir( config.dirName )
   
   os.system('killall -9 eqPly')
   os.system('cexec killall -9 eqPly')

   oldDir = os.getcwd()
      
   subDirName = config.dirName + "/" + str( serverCount )   
   if not os.path.exists( subDirName ):
      os.mkdir( subDirName )
      
   os.chdir( subDirName )
   roiStr = ''   
   if( config.roiState == False ):
      roiStr = ' -d '

   eqLayoutArg = '--eq-layout "%s" ' % ( config.layoutName )
   eqPlyConfigArg = '--eq-config "%s" ' % ( config.session )
   nbOfFramesArg = '-n ' + str(config.nbOfFrames) 

   startServers.startServers( 1, config.serverCount, config.session )
   cmdStr = eqPlyBinaryPath + ' ' + eqPlyConfigArg + ' ' + eqPlyDefaultArgs + ' ' + roiStr + ' ' + eqLayoutArg + ' ' + nbOfFramesArg

   print cmdStr

   os.system( cmdStr )
   os.chdir( oldDir)      

def main():
    testScheme( "eqPly", test )

if __name__ == "__main__":
    main()

