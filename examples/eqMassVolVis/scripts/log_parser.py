
#!/usr/bin/python

import sys
import inspect

class Stat(object):
    def __init__(self):
        self.frameNumber     = -1
        self.timeFrame       = 0
        self.timeRendering   = 0
        self.blocksRendered  = 0
        self.timeBlocksToCPU = 0
        self.timeBlocksToGPU = 0
        self.timeBlocksReloadedOnGPU = 0
        self.blocksToCPU     = 0
        self.blocksToGPU     = 0
        self.blocksReloadedOnGPU = 0

    def header(self):
        return ", ".join( [str(s) for s in vars( self ).keys()  ] )

    def values(self):
        return ", ".join( [str(s) for s in vars( self ).values()] )



if len(sys.argv) < 2:
    print "Please give a file name as a parameter"
    sys.exit()

filename = sys.argv[1]

line, fileIn = "", ""
try:
    fileIn = open( filename, "r" )
except IOError:
    print "Can't open %s for reading " % filename
    sys.exit()

line = fileIn.readline()

curFrame = Stat()
print curFrame.header()

while line:
    #print line,
    symbols = [s for s in line.split(" ") if s ]
    sLen = len( symbols )

    if symbols[1] == "Model":

        if symbols[6] == "NF":
            if curFrame.frameNumber >= 0:
                print curFrame.values()
            curFrame = Stat()
            curFrame.frameNumber = int(symbols[8])

        elif symbols[6] == "TFT":
            curFrame.timeFrame = float(symbols[8])

        elif symbols[6] == "RB":
            curFrame.blocksRendered +=   int(symbols[ 8])
            curFrame.timeRendering  += float(symbols[10])

    elif symbols[1] == "RAM_Loader":
        if symbols[6] == "RBL":
            curFrame.blocksToCPU += 1
            curFrame.timeBlocksToCPU = float(symbols[10])

    elif symbols[1] == "GPU_Loader":
        if symbols[6] == "GBL":
            curFrame.blocksToGPU += 1
            curFrame.timeBlocksToGPU = float(symbols[10])
        elif symbols[6] == "GBR":
            curFrame.blocksReloadedOnGPU += 1
            curFrame.timeBlocksReloadedOnGPU = float(symbols[10])

    line= fileIn.readline()

if curFrame.frameNumber >= 0:
    print curFrame.values()

print curFrame.header()


