#!/usr/bin/python

import os

numberOfServers = 13
excludedServers = [] 

protocols = [ 'TenGig', 'IPoIB', 'SDP', 'RDMA' ]  
layoutNames = [ 'Static2D', 'Dynamic2D', 'StaticDB' ]
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
roiStateList = [ 'ROIDisabled', 'ROIEnabled' ]
affStateList = [ 'NoAffinity', 'GoodAffinity', 'BadAffinity' ]

testFileName = "FPSInfo.txt"
resultsDir = "Results"

dirStack = []

class Configuration:
   dirName = ''
   ethType = 'TenGig'
   layoutName = 'Static2D'  
   roiState = 'ROIDisabled'
   affState = 'NoAffinity'
   protocol = 'TCP'
   session = ''
   nbOfFrames = 2500
   serverCount = 1
   
def saveCurrentDir():
   dirStack.append( os.getcwd() )
   
def gotoPreviousDir():
   os.chdir( dirStack.pop() ) 

def testScheme( application, function ):

   if application == "eqPly":
      for serverCount in range( 1, numberOfServers + 1 ):
         servers = findInactiveXServers( False )
         if len( servers ) > 0:
            print "Problem starting gpu_sd in cluster in nodes: " + str( servers )
         for protocol in protocols:
              for layoutName in layoutNames:
                 for roiState in roiStateList:
                    for affState in affStateList:
                        config = Configuration()
                        config.dirName = '%s-%s-%s-%s' % ( protocol, connectionType, layoutName, roiState, affState )
                        config.ethType = ethType
                        config.layoutName = layoutName
                        config.roiState = roiState
                        config.affState = affState
                        config.session = '%s-%s-%s' % ( affState, ethType, connectionType )
                        config.serverCount = serverCount
      function( config )
