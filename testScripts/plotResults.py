#!/usr/bin/python

import os
import numpy
from pylab import *

from common import *

resultsDict = dict()

def readResultsToDict( dirName, layoutName, ROIenabled, affinityState, sessionName ):

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

def plotIndividualResults( dirName, layoutName, ROIenabled, affinityState, sessionName ):

   figure()
   drawFigure( dirName, '' )
   title( dirName )
   plt.savefig( resultsDir + "/" + dirName + ".png", dpi=300)
 
   
def plotROIEnabledResults( dirName, layoutName, ROIenabled, affinityState, sessionName ):
   
   if not ROIenabled:
      return
  
   labelName = layoutName + "," + affStateStr[ affinityState ] 
   drawFigure( dirName, labelName )
   
def plotROIDisabledResults( dirName, layoutName, ROIenabled, affinityState, sessionName ):
   
   if ROIenabled:
      return
      
   labelName = layoutName + "," + affStateStr[ affinityState ] 
   drawFigure( dirName, labelName )
   
def plotAffinityEnabledResults( dirName, layoutName, ROIenabled, affinityState, sessionName ):
   
   if not affStateStr[ affinityState ]  == "AffinityEnabled":
      return
      
   labelName = layoutName + "," + roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName ) 

def plotAffinityDisabledResults( dirName, layoutName, ROIenabled, affinityState, sessionName ):
   
   if not affStateStr[ affinityState ]  == "AffDisabled":
      return
      
   labelName = layoutName + "," + roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName )
   
def plotWrongAffinityEnabledResults( dirName, layoutName, ROIenabled, affinityState, sessionName ):
   
   if not affStateStr[ affinityState ]  == "WrongAffEnabled":
      return
      
   labelName = layoutName + "," + roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName )

def plotStatic2DResults( dirName, layoutName, ROIenabled, affinityState, sessionName ):
   
   if not( layoutName == layoutNames[ 0 ] ):
      return
      
   labelName = affStateStr[ affinityState ] + "," + roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName )
   
def plotDynamic2DResults( dirName, layoutName, ROIenabled, affinityState, sessionName ):
   
   if not( layoutName == layoutNames[ 1 ] ):
      return
      
   labelName = affStateStr[ affinityState ] + "," + roiStateStr[ int(ROIenabled) ]
   drawFigure( dirName, labelName )
   
   
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

