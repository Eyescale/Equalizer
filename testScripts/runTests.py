#!/usr/bin/python

import startServers
import os

layoutNames = [ 'Static 2D', 'Dynamic 2D' ] 
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
roiStateStr = [ 'ROIDisabled', 'ROIEnabled' ]
affStateStr = [ 'AffDisabled', 'AffEnabled' ]
nbOfFrames = 2500
nbOfFramesArg = '-n ' + str(nbOfFrames) 

def test( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not os.path.exists( dirName ):
         os.mkdir( dirName )
   
   for serverCount in range( 1, startServers.numberOfServers + 1 ):
   
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


def testScheme( function ):
   for layoutName in layoutNames:
       for roiState in range(0, len(roiStateStr)):
         for affState in range(0, len(affStateStr)):
            dirName =  '%s-%s-%s' % (layoutName, roiStateStr[roiState], affStateStr[affState])
            function( dirName, layoutName, bool(roiState), bool(affState), affStateStr[affState] )
       

def main():
    testScheme( test )

if __name__ == "__main__":
    main()

