#!/usr/bin/python

import startServers
import os

layoutNames = [ 'Static 2D', 'Dynamic 2D' ] 
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
roiStateStr = [ 'ROIDisabled', 'ROIEnabled' ]
affStateStr = [ 'AffDisabled', 'AffEnabled' ]
nbOfFrames = 2000
nbOfFramesArg = '-n ' + str(nbOfFrames) 

def test( dirName, layoutName, ROIenabled, affinityEnabled, numberOfServers, sessionName ):
   
   os.system('killall -9 eqPly')
   os.system('cexec killall -9 eqPly')
   
   oldDir = os.getcwd()
   if not os.path.exists( dirName ):
      os.mkdir( dirName )
      
   subDirName = dirName + "/" + str(numberOfServers)   
   if not os.path.exists( subDirName ):
      os.mkdir( subDirName )
      
   os.chdir( subDirName )
   roiStr = ''   
   if( ROIenabled == False ):
      roiStr = ' -d '

   eqLayoutArg = '--eq-layout "%s" ' % ( layoutName )
   eqPlyConfigArg = '--eq-config "%s" ' % ( sessionName )
   
   startServers.startServers( 1, numberOfServers, sessionName )
   cmdStr = eqPlyBinaryPath + ' ' + eqPlyConfigArg + ' ' + eqPlyDefaultArgs + ' ' + roiStr + ' ' + eqLayoutArg + ' ' + nbOfFramesArg
  
   print cmdStr
   os.system( cmdStr )
   # os.system('echo "%s" > test.txt' % cmdStr )
      
   os.chdir( oldDir)

for i in range( 1, 14 ):
   for layoutName in layoutNames:
       for roiState in range(0, len(roiStateStr)):
         for affState in range(0, len(affStateStr)):
            dirName =  '%s-%s-%s' % (layoutName, roiStateStr[roiState], affStateStr[affState])
            test( dirName, layoutName, bool(roiState), bool(affState), i, affStateStr[affState] )
            # test( dirName, layoutName, bool(roiState), bool(affState), i, affStateStr[1] )

   

