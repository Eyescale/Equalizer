#!/usr/bin/python

numberOfServers = 13
excludedServers = [] 

protocols = [ 'TenGig', 'IPoIB', 'SDP', 'RDMA' ]  
layoutNames = [ 'Static2D', 'Dynamic2D', 'StaticDB' ]
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
roiStateStr = [ 'ROIDisabled', 'ROIEnabled' ]
affStateStr = [ 'NoAffinity', 'GoodAffinity', 'BadAffinity' ]

testFileName = "FPSInfo.txt"
resultsDir = "Results"

class Configuration:
   dirName = ''
   ethType = 'TenGig'
   layoutName = 'Static2D'  
   roiState = False 
   affState = 0 #'NoAffinity'
   protocol = 'TCP'
   session = ''
   nbOfFrames = 2500
   serverCount = 1

def testScheme( application, function ):

   if application == "eqPly":
      for serverCount in range( 1, numberOfServers + 1 ):
         for protocol in protocols
              for layoutName in layoutNames:
                 for roiState in range(0, len( roiStateStr )):
                    for affState in range(0, len( affStateStr )):
                        config = Configuration()
                        config.dirName = '%s-%s-%s-%s' % ( protocol, connectionType, layoutName, roiStateStr[roiState], affStateStr[affState] )
                        config.ethType = ethType
                        config.layoutName = layoutName
                        config.roiState = bool(roiState)
                        config.affState = affState
                        config.session = '%s-%s-%s' % ( affStateStr[ affState ], ethType, connectionType )
                        config.serverCount = serverCount
      function( config )
