#!/usr/bin/python

import os
import numpy
from pylab import *

from common import *

resultsDict = dict()

def readResultsToDict( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):

   if not os.path.exists( resultsDir ):
      convertTestResultsToCSV.main()
      
   oldDir = os.getcwd()  
   os.chdir( resultsDir )
   data = numpy.genfromtxt( dirName + ".txt", dtype=None )
   resultsDict[ dirName ] = data
   os.chdir( oldDir )

def drawFigure( dirName, labelName ):
     
   xarr = arange( numberOfServers ) + 1 
   data = resultsDict[ dirName ]
   p = plot( xarr, data, label = labelName ) 
   legend( loc=2 )
   xlabel('Number of nodes')
   ylabel('FPS') 
   return p

def plotIndividualResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):

   figure()
   drawFigure( dirName, '' )
   title( dirName )
   plt.savefig( resultsDir + "/" + dirName + ".png", dpi=300)
 
   
def plotROIEnabledResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not ROIenabled:
      return
  
   labelName = layoutName + "," + affStateStr[ int(affinityEnabled) ] 
   drawFigure( dirName, labelName )
   
def plotROIDisabledResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if ROIenabled:
      return
      
   labelName = layoutName + "," + affStateStr[ int(affinityEnabled) ] 
   drawFigure( dirName, labelName )
   
def plotAffinityEnabledResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not affinityEnabled:
      return
      
   labelName = layoutName + "," + roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName ) 

def plotAffinityDisabledResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if affinityEnabled:
      return
      
   labelName = layoutName + "," + roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName )
   
def plotStatic2DResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not( layoutName == layoutNames[ 0 ] ):
      return
      
   labelName = affStateStr[ int(affinityEnabled) ] + "," + roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName )
   
def plotDynamic2DResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not( layoutName == layoutNames[ 1 ] ):
      return
      
   labelName = affStateStr[ int(affinityEnabled) ] + "," + roiStateStr[ int(ROIenabled) ]
   drawFigure( dirName, labelName )
   
   
def main():
     
    testScheme( readResultsToDict )
    
    testScheme( plotIndividualResults )
    # show()
    
    figure()
    testScheme( plotROIDisabledResults )
    title( "ROIDisabled" )
    plt.savefig( resultsDir + "/ROIDisabled.png", dpi=300)
    # show()

    figure()
    testScheme( plotROIEnabledResults )
    title( "ROIEnabled" )
    plt.savefig( resultsDir + "/ROIEnabled.png", dpi=300)
    # show()
    
    figure()
    testScheme( plotAffinityDisabledResults )
    title( "AffinityEnabled" )
    plt.savefig( resultsDir + "/AffinityDisabled.png", dpi=300)
    # show()

    figure()
    testScheme( plotAffinityEnabledResults )
    title( "AffinityEnabled" )
    plt.savefig( resultsDir + "/AffinityEnabled.png", dpi=300)
    # show()
    
    figure()
    testScheme( plotStatic2DResults )
    title( layoutNames[ 0 ] )
    plt.savefig( resultsDir + "/" + layoutNames[ 0 ] + ".png", dpi=300)
    # show()

    figure()
    testScheme( plotDynamic2DResults )
    title( layoutNames[ 1 ] )
    plt.savefig( resultsDir + "/" + layoutNames[ 1 ] + ".png", dpi=300)
    # show()


if __name__ == "__main__":
    main()

