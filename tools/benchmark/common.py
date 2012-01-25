#!/usr/bin/python

import os
import checkXServersAndRestart

numberOfServers = 13
excludedServers = [] 

# protocols = [ 'TenGig', 'IPoIB', 'RDMA', 'SDP' ]  
protocols = [ 'TenGig', 'IPoIB' ]  
eqPlyLayoutNames = [ 'StaticDB', 'Static2D', 'Dynamic2D' ]
rtNeuronLayoutNames = [ 'RoundRobinDB', 'SpatialDB', 'Static2D', 'Dynamic2D' ]
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
rtNeuromBinaryPath = '/home/bilgili/Build/bin/rtneuron.equalizer'

eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
rtNeuronDefaultArgs = '-b /home/bilgili/RTNeuronData/blueconfig --target mc0_Column mesh none 1 0 0 1.0 --no-sim-data  --no-lod  --no-selections  --background 1.0 1.0 1.0 1.0'
roiStateList = [  'ROIDisabled' ]
affStateList = [  'GoodAffinity', 'BadAffinity', 'NoAffinity' ]

testFileName = "FPS.eqPly.txt"
gpuCountFile = "GPUCount.txt"
gpuCountFPSFile = "GPUCountFPS.txt"

timeSecToWaitForProcess = 20 * 60 # Wait for 20 minutes before killing process ( possible hang )

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
           for layoutName in eqPlyLayoutNames:
              for roiState in roiStateList:
                 for affState in affStateList:
                     config = Configuration()
                     config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
                     config.protocol = protocol
                     config.layoutName = layoutName
                     config.roiState = roiState
                     config.affState = affState
                     config.session = '%s-%s-%s' % ( application, affState, protocol )
                     config.serverCount = serverCount
                     function( config )
                     
                     
   if application == "rtneuron":
      #servers = checkXServersAndRestart.findInactiveXServers( False )
      #if len( servers ) > 0:
      #  print "Problem starting gpu_sd in cluster in nodes: " + str( servers )
      #  exit()
      for protocol in protocols:
           for layoutName in rtNeuronLayoutNames:
              for roiState in roiStateList:
                 for affState in affStateList:
                     config = Configuration()
                     config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
                     config.protocol = protocol
                     config.layoutName = layoutName
                     config.roiState = roiState
                     config.affState = affState
                     config.session = '%s-%s-%s' % ( application, affState, protocol )
                     config.serverCount = serverCount
                     function( config )                  
