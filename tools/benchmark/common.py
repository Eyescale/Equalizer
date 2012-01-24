#!/usr/bin/python

import os
import checkXServersAndRestart

numberOfServers = 13
excludedServers = [] 

protocols = [ 'TenGig', 'IPoIB', 'RDMA', 'SDP' ]  
layoutNames = [ 'StaticDB', 'Static2D', 'Dynamic2D' ]
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
roiStateList = [  'ROIEnabled', 'ROIDisabled' ]
affStateList = [  'GoodAffinity', 'BadAffinity', 'NoAffinity' ]

testFileName = "FPS.eqPly.txt"
gpuCountFile = "GPUCount.txt"
gpuCountFPSFile = "GPUCountFPS.txt"

timeSecToWaitForProcess = 15 * 60 # Wait for 15 minutes before killing process ( possible hang )

dirStack = []

class Configuration:
   dirName = ''
   protocol = 'TenGig'
   layoutName = 'Static2D'  
   roiState = 'ROIDisabled'
   affState = 'NoAffinity'
   protocol = 'TCP'
   session = ''
   nbOfFrames = 2020
   serverCount = 1
   
   
def saveCurrentDir():
   dirStack.append( os.getcwd() )
   
def gotoPreviousDir():
   os.chdir( dirStack.pop() ) 

def testScheme( application, function, serverCount ):
   if application == "eqPly":
      #servers = checkXServersAndRestart.findInactiveXServers( False )
      #if len( servers ) > 0:
      #  print "Problem starting gpu_sd in cluster in nodes: " + str( servers )
      #  exit()
      for protocol in protocols:
           for layoutName in layoutNames:
              for roiState in roiStateList:
                 for affState in affStateList:
                     config = Configuration()
                     config.dirName = '%s-%s-%s-%s' % ( protocol, layoutName, roiState, affState )
                     config.protocol = protocol
                     config.layoutName = layoutName
                     config.roiState = roiState
                     config.affState = affState
                     config.session = '%s-%s' % ( affState, protocol )
                     config.serverCount = serverCount
                     function( config )
