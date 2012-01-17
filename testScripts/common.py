#!/usr/bin/python

numberOfServers = 13
excludedServers = [] 

interfaceHostnameDict = dict()
interfaceHostnameDict[ 'TenGig' ] = 'node%st'
interfaceHostnameDict[ 'Infiniband' ] = 'node%si'

protocols = [ 'TCP', 'SDP', 'RDMA' ]  
layoutNames = [ 'Static2D', 'Dynamic2D', 'StaticDB' ]
eqPlyBinaryPath = '/home/bilgili/Build/bin/eqPly'
eqPlyDefaultArgs = '-m ~/EqualizerData/david1mm.ply -a ~/EqualizerConfigs/eqPly/cameraPath'
roiStateStr = [ 'ROIDisabled', 'ROIEnabled' ]
affStateStr = [ 'AffDisabled', 'AffEnabled' 'WrongAffEnabled' ]

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

def testScheme( application, function ):

   if application == "eqPly":
      for ethType in interfaceHostnameDict.keys():
         for layoutName in layoutNames:
             for roiState in range(0, len(roiStateStr)):
               for affState in range(0, len(affStateStr)):
                  config = Configuration()
               
                  config.dirName = '%s-%s-%s-%s' % (ethType, layoutName, roiStateStr[roiState], affStateStr[affState])
                  config.ethType = ethType
                  config.layoutName = layoutName
                  config.roiState = bool(roiState)
                  config.affState = affState
                  config.session = affStateStr[affState]

                  function( config )
