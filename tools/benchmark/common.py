#!/usr/bin/python

numberOfServers = 13
excludedServers = [] 

protocols = [ 'TCPIP', 'SDP', 'RDMA' ]  
interfaceProtocolDict = dict()
interfaceProtocolDict[ 'TenGig' ] = [ protocols[0] ]  
interfaceProtocolDict[ 'Infiniband' ] = protocols 

layoutNames = [ 'Static2D', 'Dynamic2D', 'StaticDB' ]
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
roiStateStr = [ 'ROIDisabled', 'ROIEnabled' ]
affStateStr = [ 'AffDisabled', 'AffEnabled', 'WrongAffEnabled' ]

testFileName = "FPSInfo.txt"
resultsDir = "Results"

class Configuration:
   dirName = ''
   ethType = 'TenGig'
   layoutName = 'Static2D'  
   roiState = False 
   affState = 0 #'AffDisabled'
   protocol = 'TCP'
   session = ''
   nbOfFrames = 2500
   serverCount = 1

def testScheme( application, function ):

   if application == "eqPly":
      for serverCount in range( 1, numberOfServers + 1 ):
         for ethType in interfaceProtocolDict.keys():
           for connectionType in interfaceProtocolDict[ ethType ]:
              for layoutName in layoutNames:
                 for roiState in range(0, len( roiStateStr )):
                    for affState in range(0, len( affStateStr )):
                        config = Configuration()
                        config.dirName = '%s-%s-%s-%s-%s' % ( ethType, connectionType, layoutName, roiStateStr[roiState], affStateStr[affState] )
                        config.ethType = ethType
                        config.layoutName = layoutName
                        config.roiState = bool(roiState)
                        config.affState = affState
                        config.session = '%s-%s-%s' % ( affStateStr[ affState ], ethType, connectionType )
                        config.serverCount = serverCount
      function( config )
