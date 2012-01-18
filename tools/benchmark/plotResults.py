#!/usr/bin/python

import os
import numpy
from pylab import *

from common import *

resultsDict = dict()

def readResultsToDict( config ):

   if( config.serverCount != 1 ) # Run only once
      return

   if not os.path.exists( resultsDir ):
      convertTestResultsToCSV.main()
      
   oldDir = os.getcwd()  
   os.chdir( resultsDir )
   data = numpy.genfromtxt( config.dirName + ".txt", dtype=None )
   resultsDict[ config.dirName ] = data
   os.chdir( oldDir )

def drawFigure( dirName, labelName ):
     
   if( config.serverCount != 1 ) # Run only once
      return

   xarr = arange( numberOfServers ) + 1 
   data = resultsDict[ dirName ]
   p = plot( xarr, data, label = labelName ) 
   legend( loc=2 )
   xlabel('Number of nodes')
   ylabel('FPS') 
   return p

def plotIndividualResults( config ):

   if( config.serverCount != 1 ) # Run only once
      return

   figure()
   drawFigure( config.dirName, '' )
   title( config.dirName )
   plt.savefig( resultsDir + "/" + config.dirName + ".png", dpi=300)
 
   
def plotROIEnabledResults( config ):
   
   if( config.serverCount != 1 ) # Run only once
      return
   
   if not bool( config.roiState ):
      return
  
   labelName = config.layoutName + "," + affStateStr[ config.affinityState ] 
   drawFigure( config.dirName, labelName )
   
def plotROIDisabledResults( config ):
   
   if( config.serverCount != 1 ) # Run only once
      return

   if bool( config.roiState ):
      return
      
   labelName = config.layoutName + "," + affStateStr[ config.affinityState ] 
   drawFigure( config.dirName, labelName )
   
def plotAffinityEnabledResults( config ):
   
   if( config.serverCount != 1 ) # Run only once
      return
   
   if not affStateStr[ config.affinityState ]  == affStateStr[ 1 ]:
      return
      
   labelName = config.layoutName + "," + roiStateStr[ int(config.roiState) ] 
   drawFigure( dirName, labelName ) 

def plotAffinityDisabledResults( config ):
   
    if( config.serverCount != 1 ) # Run only once
      return
   
   if not affStateStr[ config.affinityState ]  == affStateStr[ 0 ]:
      return
      
   labelName = config.layoutName + "," + roiStateStr[ int(config.roiState) ] 
   drawFigure( config.dirName, labelName )
   
def plotWrongAffinityEnabledResults( config ):
   
   if( config.serverCount != 1 ) # Run only once
      return
   
   if not affStateStr[ config.affinityState ]  == affStateStr[ 2 ]:
      return
      
   labelName = config.layoutName + "," + roiStateStr[ int(config.roiState) ] 
   drawFigure( config.dirName, labelName )

def plotStatic2DResults( config ):
   
   if( config.serverCount != 1 ) # Run only once
      return

   if not( config.layoutName == layoutNames[ 0 ] ):
      return
      
   labelName = affStateStr[ config.affinityState ] + "," + roiStateStr[ int(config.roiState) ] 
   drawFigure( config.dirName, labelName )
   
def plotDynamic2DResults( config ):
   
   if( config.serverCount != 1 ) # Run only once
      return;

   if not( config.layoutName == layoutNames[ 1 ] ):
      return
      
   labelName = affStateStr[ config.affinityState ] + "," + roiStateStr[ int(config.roiState) ]
   drawFigure( config.dirName, labelName )  
   
def main():
     
    testScheme( "eqPly", readResultsToDict )
    
    testScheme( "eqPly", plotIndividualResults )
    # show()
    
    figure()
    testScheme( "eqPly", plotROIDisabledResults )
    title( "ROIDisabled" )
    plt.savefig( resultsDir + "/ROIDisabled.png", dpi=300)
    # show()

    figure()
    testScheme( "eqPly", plotROIEnabledResults )
    title( "ROIEnabled" )
    plt.savefig( resultsDir + "/ROIEnabled.png", dpi=300)
    # show()
    
    figure()
    testScheme( "eqPly", plotAffinityDisabledResults )
    title( "AffinityDisabled" )
    plt.savefig( resultsDir + "/AffinityDisabled.png", dpi=300)
    # show()

    figure()
    testScheme( "eqPly", plotAffinityEnabledResults )
    title( "AffinityEnabled" )
    plt.savefig( resultsDir + "/AffinityEnabled.png", dpi=300)
    # show()

    figure()
    testScheme( "eqPly", plotWrongAffinityEnabledResults )
    title( "AffinityEnabled" )
    plt.savefig( resultsDir + "/WrongAffinityEnabled.png", dpi=300)
    # show()
    
    figure()
    testScheme( "eqPly", plotStatic2DResults )
    title( layoutNames[ 0 ] )
    plt.savefig( resultsDir + "/" + layoutNames[ 0 ] + ".png", dpi=300)
    # show()

    figure()
    testScheme( "eqPly", plotDynamic2DResults )
    title( layoutNames[ 1 ] )
    plt.savefig( resultsDir + "/" + layoutNames[ 1 ] + ".png", dpi=300)
    # show()


if __name__ == "__main__":
    main()

