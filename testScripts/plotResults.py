#!/usr/bin/python

import startServers
import os
import numpy
import runTests
import convertTestResultsToCSV
from pylab import *

resultsDict = dict()

def readResultsToDict( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):

   if not os.path.exists( convertTestResultsToCSV.resultsDir ):
      convertTestResultsToCSV.main()
      
   oldDir = os.getcwd()  
   os.chdir( convertTestResultsToCSV.resultsDir )
   data = numpy.genfromtxt( dirName + ".txt", dtype=None )
   resultsDict[ dirName ] = data
   os.chdir( oldDir )

def drawFigure( dirName, labelName ):
     
   xarr = arange( startServers.numberOfServers ) + 1 
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
   plt.savefig( convertTestResultsToCSV.resultsDir + "/" + dirName + ".png", dpi=300)
 
   
def plotROIEnabledResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not ROIenabled:
      return
  
   labelName = layoutName + "," + runTests.affStateStr[ int(affinityEnabled) ] 
   drawFigure( dirName, labelName )
   
def plotROIDisabledResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if ROIenabled:
      return
      
   labelName = layoutName + "," + runTests.affStateStr[ int(affinityEnabled) ] 
   drawFigure( dirName, labelName )
   
def plotAffinityEnabledResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not affinityEnabled:
      return
      
   labelName = layoutName + "," + runTests.roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName ) 

def plotAffinityDisabledResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if affinityEnabled:
      return
      
   labelName = layoutName + "," + runTests.roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName )
   
def plotStatic2DResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not( layoutName == runTests.layoutNames[ 0 ] ):
      return
      
   labelName = runTests.affStateStr[ int(affinityEnabled) ] + "," + runTests.roiStateStr[ int(ROIenabled) ] 
   drawFigure( dirName, labelName )
   
def plotDynamic2DResults( dirName, layoutName, ROIenabled, affinityEnabled, sessionName ):
   
   if not( layoutName == runTests.layoutNames[ 1 ] ):
      return
      
   labelName = runTests.affStateStr[ int(affinityEnabled) ] + "," + runTests.roiStateStr[ int(ROIenabled) ]
   drawFigure( dirName, labelName )
   
   
def main():
     
    runTests.testScheme( readResultsToDict )
    
    runTests.testScheme( plotIndividualResults )
    # show()
    
    figure()
    runTests.testScheme( plotROIEnabledResults )
    title( "ROIDisabled" )
    plt.savefig( convertTestResultsToCSV.resultsDir + "/ROIDisabled.png", dpi=300)
    # show()
    
    figure()
    runTests.testScheme( plotAffinityEnabledResults )
    title( "ROIEnabled" )
    plt.savefig( convertTestResultsToCSV.resultsDir + "/ROIEnabled.png", dpi=300)
    # show()

    figure()
    runTests.testScheme( plotAffinityDisabledResults )
    title( "AffinityDisabled" )
    plt.savefig( convertTestResultsToCSV.resultsDir + "/AffinityDisabled.png", dpi=300)
    # show()
    
    figure()
    runTests.testScheme( plotROIDisabledResults )
    title( "AffinityEnabled" )
    plt.savefig( convertTestResultsToCSV.resultsDir + "/AffinityEnabled.png", dpi=300)
    # show()
    
    figure()
    runTests.testScheme( plotStatic2DResults )
    title( runTests.layoutNames[ 0 ] )
    plt.savefig( convertTestResultsToCSV.resultsDir + "/" + runTests.layoutNames[ 0 ] + ".png", dpi=300)
    # show()

    figure()
    runTests.testScheme( plotDynamic2DResults )
    title( runTests.layoutNames[ 1 ] )
    plt.savefig( convertTestResultsToCSV.resultsDir + "/" + runTests.layoutNames[ 1 ] + ".png", dpi=300)
    # show()


if __name__ == "__main__":
    main()

