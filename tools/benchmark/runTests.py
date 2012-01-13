#!/usr/bin/python

import startServers
from common import *
import os

def test( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not os.path.exists( dirName ):
         os.mkdir( dirName )
   
   for serverCount in range( 1, numberOfServers + 1 ):
   
      os.system('killall -9 eqPly')
      os.system('cexec killall -9 eqPly')
      
      oldDir = os.getcwd()
         
      subDirName = dirName + "/" + str( serverCount )   
      if not os.path.exists( subDirName ):
         os.mkdir( subDirName )
         
      os.chdir( subDirName )
      roiStr = ''   
      if( ROIenabled == False ):
         roiStr = ' -d '

      eqLayoutArg = '--eq-layout "%s" ' % ( layoutName )
      eqPlyConfigArg = '--eq-config "%s" ' % ( sessionName )
      
      startServers.startServers( 1, serverCount, sessionName )
      cmdStr = eqPlyBinaryPath + ' ' + eqPlyConfigArg + ' ' + eqPlyDefaultArgs + ' ' + roiStr + ' ' + eqLayoutArg + ' ' + nbOfFramesArg
     
      print cmdStr
   
      os.system( cmdStr )
      os.chdir( oldDir)      

def main():
    testScheme( test )

if __name__ == "__main__":
    main()

