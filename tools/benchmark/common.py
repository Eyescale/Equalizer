#!/usr/bin/python

import os
import checkXServersAndRestart

numberOfServers = 11
excludedServers = [ 5, 10 ] 

# All test options
protocolsFull = [ 'TenGig', 'IPoIB', 'RDMA', 'SDP' ]  
affFullStateList = [  'GoodAffinity', 'BadAffinity', 'NoAffinity' ]
rtNeuronFullLayoutNames = [ 'Static2D', 'RoundRobinDB', 'SpatialDB','Dynamic2D', 'DBDirectSendSDB', 'DBDirectSendRR', 'DB_2DSDB', 'DB_2DRR' ]
eqPlyFullLayoutNames = [ 'StaticDB', 'Static2D', 'Dynamic2D', 'DB_2D', 'DBDirectSend' ]
roiFullStateList = [  'ROIEnabled', 'ROIDisabled' ]

eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
rtNeuromBinaryPath = '/home/bilgili/Build/bin/rtneuron.equalizer'
eqPlyDefaultArgs = '-a ~/EqualizerConfigs/eqPly/cameraPath --eq-logfile node01.txt'
rtNeuronDefaultArgs = '--profile  --path-fps 20  -b /home/bilgili/RTNeuronData/blueconfig --no-sim-data --no-selections  --background 0.5 0.5 0.5 1.0 --path ~/RTNeuronData/rotation.path --eq-logfile node01.txt'

testFileName = "FPS.eqPly.txt"
gpuCountFile = "GPUCount.txt"
gpuCountFPSFile = "GPUCountFPS.txt"
gpuCountHFPSFile = "GPUCountHFPS.txt"
gpuCountLFPSFile = "GPUCountLFPS.txt"
commandFile = "CommandString"
rtneuronFPSFile = "statistics.txt"
optionsDumpFilename = "runoptions.obj"
emptyLayout = "notset"

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
   forceRedo = False
   checkAndRedo = False
    
def saveCurrentDir():
   dirStack.append( os.getcwd() )
   
def gotoPreviousDir():
   os.chdir( dirStack.pop() ) 

def writeCommandStringToFile( cmdStr ):
   f = open( commandFile, 'w' )
   f.write( cmdStr )
   f.close( )

def eqPlysingleTestScheme( application, function, serverCount ):

   # run tests options
   protocol = 'TenGig' 
   layoutName = 'DBDirectSend'
   roiState = "ROIDisabled"
   affState = "GoodAffinity"
   
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

def eqPlyflagsTestScheme( application, function, serverCount ):

   # run tests options
   protocols = [ 'TenGig' ]  
   eqPlyLayoutNames = [ 'DBDirectSend' ]
   
   for protocol in protocols:
      for layoutName in eqPlyLayoutNames:
         
         config = Configuration()
         config.protocol = protocol
         config.layoutName = layoutName
         config.serverCount = serverCount
         config.nbOfFrames = 1210
         
         roiState = "ROIEnabled"
         affState = "NoAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         
         function( config )
      
         roiState = "ROIEnabled"
         affState = "NoAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         
         # function( config )
         
         roiState = "ROIDisabled"
         affState = "GoodAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         
         # function( config )
         
         roiState = "ROIDisabled"
         affState = "BadAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         
         # function( config )


def eqPlyfullTestScheme( application, function, serverCount ):

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

   # run tests options
   protocols = [ 'TenGig' ]  
   eqPlyLayoutNames = [ 'DBDirectSend' ]
   roiStateList = [  'ROIDisabled' ]
   affStateList = [  'GoodAffinity' ]
   
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


def eqPlynetworkTestScheme( application, function, serverCount ):

   # run tests options
   protocols = [ 'SDP', 'RDMA', 'IPoIB' ]  
   eqPlyLayoutNames = [ 'DBDirectSend' ]
   roiStateList = [  'ROIDisabled' ]
   affStateList = [  'GoodAffinity' ]
   
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

   protocol = 'TenGig'
   layoutName = 'DBDirectSendSDB'
   roiState = "ROIDisabled"
   affState = "GoodAffinity"
   
   config = Configuration()
   config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
   print config.dirName
   config.protocol = protocol
   config.layoutName = layoutName
   config.roiState = roiState
   config.affState = affState
   config.session = '%s-%s-%s' % ( application, affState, protocol )
   config.serverCount = serverCount
   config.nbOfFrames = 400
   function( config )



def rtneuroncombinationTestScheme( application, function, serverCount ):

   # run tests options
   protocols = [ 'TenGig' ]  
   rtNeuronLayoutNames = [ 'Static2D', 'DBDirectSendSDB', 'DBDirectSendRR' ]
   roiStateList = [  'ROIDisabled' ]
   affStateList = [  'GoodAffinity' ]

   for protocol in protocols:
     for layoutName in rtNeuronLayoutNames:
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

def rtneuronflagsTestScheme( application, function, serverCount ):

   # run tests options
   protocols = [ 'TenGig' ]  
   rtNeuronLayoutNames = [  'Dynamic2D' ]
   
   config = Configuration()
   config.serverCount = serverCount
   config.nbOfFrames = 400
   
   for protocol in protocols:
      for layoutName in rtNeuronLayoutNames:
         
         config.protocol = protocol
         config.layoutName = layoutName
         
         roiState = "ROIDisabled"
         affState = "GoodAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         
         # function( config )
      
         roiState = "ROIDisabled"
         affState = "BadAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         
         # function( config )
         
         roiState = "ROIDisabled"
         affState = "GoodAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         
         # function( config )
         
         roiState = "ROIDisabled"
         affState = "BadAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         
         # function( config )

   rtNeuronLayoutNames = [  'DBDirectSendSDB', 'DBDirectSendRR' ]
   
   config = Configuration()
   config.serverCount = serverCount
   config.nbOfFrames = 400
   
   for protocol in protocols:
      for layoutName in rtNeuronLayoutNames:
         
         config.protocol = protocol
         config.layoutName = layoutName
         
         roiState = "ROIEnabled"
         affState = "NoAffinity"
         config.roiState = roiState
         config.affState = affState
         config.dirName = '%s-%s-%s-%s-%s' % ( application, protocol, layoutName, roiState, affState )
         config.session = '%s-%s-%s' % ( application, affState, protocol )
         function( config )



def rtneuronfullTestScheme( application, function, serverCount ):

   for protocol in protocolsFull:
      for layoutName in rtNeuronFullLayoutNames:
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
               config.nbOfFrames = 400
               function( config )

def testScheme( schemeName, application, function, serverCount ):

   functioName = '%s%sTestScheme' % ( application, schemeName )
   
   if( not globals().has_key( functioName ) ):
      print "No such schema function: " + functioName
      exit()
   
   schemeFunc = globals()[ functioName ]  
   schemeFunc( application, function, serverCount )
   

                
