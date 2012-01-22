#!/usr/bin/python

import startServers
from common import *
import os
import glob

def fixTests( config ):
   
   if( config.serverCount == 1 ):
      return

   if not os.path.exists( config.dirName ):
      return
   
   subDirName = config.dirName + "/" + str( config.serverCount )
   if not os.path.exists( subDirName ):
      return

   nodeCount = len( glob.glob( subDirName + "/node*.log" ) )  
   
   if ( nodeCount == config.serverCount ):
      return
  
   newSubDirName =  config.dirName + "/" + str( nodeCount )

   if os.path.exists( newSubDirName ):
      print( "Tests already exists for node count: " + str( nodeCount ) )
      return

   # os.rename( subDirName,  newSubdirName )
  
   print subDirName + "  " + newSubDirName + " " + str( nodeCount )

def main():
    testScheme( "eqPly", fixTests )

if __name__ == "__main__":
    main()
