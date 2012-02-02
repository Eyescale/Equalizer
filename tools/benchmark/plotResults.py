#!/usr/bin/python

import os
import numpy
from pylab import *
from optparse import OptionParser

from common import *

resultsDict = dict()

keyConfiguration = Configuration()

def readResultsToDict( config ):

   gpuCountFPSFilePath = config.dirName + "/" + gpuCountFPSFile
   
   if( not os.path.exists( gpuCountFPSFilePath ) ):
      return

   data = numpy.genfromtxt( gpuCountFPSFilePath, dtype=None )
   resultsDict[ config.dirName ] = data
  
def drawFigure( dirName, labelName ):
  
   if( not resultsDict.has_key( dirName ) ):
      return
  
   xarr = []
   data = []
   
   for item in resultsDict[ dirName ]:
      xarr.append( item[ 0 ] )
      data.append( item[ 1 ] )
  
   p = plot( xarr, data, label = labelName ) 
   legend( loc = 0 )
   xlabel('Number of GPUs')
   ylabel('FPS') 
   return p
   

def plotIndividualResults( config ):

   if( not resultsDict.has_key( config.dirName ) ):
      return
  
   figure()
   drawFigure( config.dirName, '' )
   title( config.dirName )
   plt.savefig( config.dirName + ".png", dpi=300)
   
def plotLayoutROIStateResults( config ):
   
   if( not resultsDict.has_key( config.dirName ) ):
      return
   
   if not ( config.layoutName == keyConfiguration.layoutName and config.roiState == keyConfiguration.roiState ):
      return
  
   labelName = config.protocol + "," + config.affState
   drawFigure( config.dirName, labelName )
   
   
def plotLayoutAffStateResults( config ):
   
   if( not resultsDict.has_key( config.dirName ) ):
      return
   
   if not ( config.layoutName == keyConfiguration.layoutName and config.affState == keyConfiguration.affState ):
      return
  
   labelName = config.protocol + "," + config.roiState
   drawFigure( config.dirName, labelName )
 
 
def plotROIStateAffStateResults( config ):
   
   if( not resultsDict.has_key( config.dirName ) ):
      return
   
   if not ( config.roiState == keyConfiguration.roiState and config.affState == keyConfiguration.affState ) :
      return
  
   labelName = config.protocol + "," + config.layoutName
   drawFigure( config.dirName, labelName )


def plotLayoutROIStateAffStateResults( config ):
   
   if( not resultsDict.has_key( config.dirName ) ):
      return
   
   if not ( config.roiState == keyConfiguration.roiState and config.layoutName == keyConfiguration.layoutName and config.affState == keyConfiguration.affState ) :
      return
  
   labelName = config.protocol
   drawFigure( config.dirName, labelName )
   
   
def plotROIStateAffStateProtocolResults( config ):
   
   if( not resultsDict.has_key( config.dirName ) ):
      return
   
   if not ( config.roiState == keyConfiguration.roiState and config.protocol == keyConfiguration.protocol and config.affState == keyConfiguration.affState ) :
      return
  
   labelName = config.layoutName
   drawFigure( config.dirName, labelName )
   

   
def plotResults( application ):
     
    testScheme( application, readResultsToDict, 1  )
    
    testScheme( application, plotIndividualResults, 1 )
    # show()
    
    layoutName = eqPlyLayoutNames
    if( application == "eqPly" ):
	layoutNames = eqPlyLayoutNames
    elif( application == "rtneuron" ):
        layoutNames == rtneuronLayoutNames
	
    for layout in layoutNames:
      for roiState in roiStateList:
         figure()
         keyConfiguration.layoutName = layout
         testScheme( application, plotLayoutROIStateResults, 1 )
         figureHeading = layout + "-" + roiState
         title( figureHeading )
         plt.savefig( figureHeading + ".png", dpi=300)
         # show()
    
    for layout in layoutNames:
      for affState in affStateList:
         figure()
         keyConfiguration.layoutName = layout
         testScheme( application, plotLayoutAffStateResults, 1 )
         figureHeading = layout + "-" + affState
         title( figureHeading )
         plt.savefig( figureHeading + ".png", dpi=300)
         # show()
    
    
    for layout in layoutNames:
      for roiState in roiStateList:
         for affState in affStateList:
            figure()
            keyConfiguration.layoutName = layout
            keyConfiguration.roiState = roiState
            keyConfiguration.affState = affState
            testScheme( application, plotLayoutROIStateAffStateResults, 1 )
            figureHeading = layout + "-" + roiState + "-" + affState
            title( figureHeading )
            plt.savefig( figureHeading + ".png", dpi=300)
            # show()
    
      for roiState in roiStateList:
         for affState in affStateList:
            figure()
            keyConfiguration.roiState = roiState
            keyConfiguration.affState = affState
            testScheme( application, plotROIStateAffStateResults, 1 )
            figureHeading = roiState + "-" + affState
            title( figureHeading )
            plt.savefig( figureHeading + ".png", dpi=300)
            # show()
    
    for protocol in protocols:
      for roiState in roiStateList:
         for affState in affStateList:
            figure()
            keyConfiguration.protocol = protocol
            keyConfiguration.roiState = roiState
            keyConfiguration.affState = affState
            testScheme( application, plotROIStateAffStateProtocolResults, 1 )
            figureHeading = protocol + "," + roiState + "," + affState 
            title( figureHeading )
            plt.savefig(  figureHeading + ".png", dpi=300)
            # show()
    
    
    return

if __name__ == "__main__":
    
   parser = OptionParser()
   parser.add_option("-a", "--application", dest="application",help="Select app ( eqPly, rtneuron )", default="eqPly")
   (options, args) = parser.parse_args()
   plotResults( options.application )
    

