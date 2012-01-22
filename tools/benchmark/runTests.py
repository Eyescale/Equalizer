#!/usr/bin/python

import startServers
import sys 

from common import *
import os

def test( config ):
   
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

def main():

    if( len( sys.argv ) == 2 ):
	if( sys.argv[1] == "FULLSCREEN" or sys.argv[1] == "fullscreen" or sys.argv[1] == '-f' ):
		os.environ['EQ_WINDOW_IATTR_HINT_FULLSCREEN'] = '1'
    else:
        os.environ['EQ_WINDOW_IATTR_HINT_FULLSCREEN'] = '0'

    testScheme( "eqPly", test )

if __name__ == "__main__":
    main()

