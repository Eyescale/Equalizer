#!/usr/bin/python

import os
import checkXServersAndRestart

numberOfServers = 13
excludedServers = [] 

# All test options
protocolsFull = [ 'TenGig', 'IPoIB', 'RDMA', 'SDP' ]  
affFullStateList = [  'GoodAffinity', 'BadAffinity', 'NoAffinity' ]
rtNeuronFullLayoutNames = [ 'RoundRobinDB', 'SpatialDB','Dynamic2D' ]
eqPlyFullLayoutNames = [ 'StaticDB', 'Static2D', 'Dynamic2D' ]
roiFullStateList = [  'ROIEnabled', 'ROIDisabled' ]

# run tests options
protocols = [ 'TenGig' ]  
eqPlyLayoutNames = [ 'Dynamic2D', 'StaticDB' ]
rtNeuronLayoutNames = [ 'SpatialDB', 'RoundRobinDB','Dynamic2D' ]
roiStateList = [  'ROIEnabled', 'ROIDisabled' ]
affStateList = [  'NoAffinity' ]


eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
rtNeuromBinaryPath = '/home/bilgili/Build/bin/rtneuron.equalizer'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
rtNeuronDefaultArgs = '--profile --path-fps 1 -b /home/bilgili/RTNeuronData/blueconfig --target mc0_Column mesh none 1 0 0 1.0 --no-sim-data  --no-lod --no-selections  --background 1.0 1.0 1.0 1.0 --path ~/RTNeuronData/camera_path.cam'
testFileName = "FPS.eqPly.txt"
gpuCountFile = "GPUCount.txt"
gpuCountFPSFile = "GPUCountFPS.txt"
commandFile = "CommandString"
rtneuronFPSFile = "statistics.txt"

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
   nbOfFrames = 900
   serverCount = 1
    
def saveCurrentDir():
   dirStack.append( os.getcwd() )
   
def gotoPreviousDir():
   os.chdir( dirStack.pop() ) 

def writeCommandStringToFile( cmdStr ):
   f = open( commandFile, 'w' )
   f.write( cmdStr )
   f.close( )


def eqPlysingleTestScheme( application, function, serverCount ):

   for protocol in protocols:
      for layoutName in eqPlyLayoutNames:

         config = Configuration()
         config.protocol = protocol
         config.layoutName = layoutName
         config.serverCount = serverCount
         config.nbOfFrames = 1210

         roiState = "ROIDisabled"
         affState = "NoAffinity"

         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.roiState = roiState
         config.affState = affState
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         function( config )
         
         roiState = "ROIEnabled"
         affState = "NoAffinity"

         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.roiState = roiState
         config.affState = affState
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         function( config )

         roiState = "ROIDisabled"
         affState = "GoodAffinity"

         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.roiState = roiState
         config.affState = affState
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         function( config )
         
         roiState = "ROIDisabled"
         affState = "BadAffinity"

         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.roiState = roiState
         config.affState = affState
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         function( config )


def eqPlyfullTestScheme( application, function, serverCount ):

   #servers = checkXServersAndRestart.findInactiveXServers( False )
   #if len( servers ) > 0:
   #  print "Problem starting gpu_sd in cluster in nodes: " + str( servers )
   #  exit()
   for protocol in protocolsFull:
      for layoutName in eqPlyFullLayoutNames:
         for roiState in roiFullStateList:
            for affState in affFullStateList:
               config = Configuration()
               config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
               config.protocol = protocol
               config.layoutName = layoutName
               config.roiState = roiState
               config.affState = affState
               config.session = '%s-%s-%s' % ( application, affState, protocol )
               config.serverCount = serverCount
               config.nbOfFrames = 1210
               function( config )


def eqPlycombinationTestScheme( application, function, serverCount ):

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
               config.nbOfFrames = 1210
               function( config )

def rtneuronsingleTestScheme( application, function, serverCount ):

   hello = None

def rtneuroncombinationTestScheme( application, function, serverCount ):

    for protocol in protocols:
      for layoutName in rtNeuronLayoutNames:
         # if ( layoutName == 'RoundRobinDB' or layoutName == 'SpatialDB' ):
         #    os.environ['EQ_NODE_IATTR_THREAD_MODEL'] = str( -10 ) # ASYNC mode
         # else:
         #    os.environ['EQ_NODE_IATTR_THREAD_MODEL'] = str( -10 ) # DRAW_SYNC mode
         for roiState in roiStateList:
            for affState in affStateList:
               config = Configuration()
               config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
               config.protocol = protocol
               config.roiState = roiState
               config.layoutName = layoutName
               config.affState = affState
               config.session = '%s-%s-%s' % ( application, affState, protocol )
               config.serverCount = serverCount
               config.nbOfFrames = 400
               function( config )       


def testScheme( schemeName, application, function, serverCount ):

   functioName = '%s%sTestScheme' % ( application, schemeName )
   
   if( not globals().has_key( functioName ) ):
      print "No such schema function: " + functioName
      exit()
   
   schemeFunc = globals()[ functioName ]  
   schemeFunc( application, function, serverCount )
   

                
