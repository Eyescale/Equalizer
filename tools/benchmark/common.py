#!/usr/bin/python

numberOfServers = 13
excludedServers = [] 

layoutNames = [ 'Static 2D', 'Dynamic 2D' ] 
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
roiStateStr = [ 'ROIDisabled', 'ROIEnabled' ]
affStateStr = [ 'AffDisabled', 'AffEnabled' ]
nbOfFrames = 2500
nbOfFramesArg = '-n ' + str(nbOfFrames) 
testFileName = "FPS.eqPly.txt"
resultsDir = "Results"

def testScheme( function ):
   for layoutName in layoutNames:
       for roiState in range(0, len(roiStateStr)):
         for affState in range(0, len(affStateStr)):
            dirName =  '%s-%s-%s' % (layoutName, roiStateStr[roiState], affStateStr[affState])
            function( dirName, layoutName, bool(roiState), bool(affState), affStateStr[affState] )
