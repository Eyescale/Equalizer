#!/usr/bin/env python
import os
import re
import sys
import threading

def getNodelist():
    nodelist = os.environ['SLURM_JOB_NODELIST']

    p = re.compile( '[^\[]+' )
    match = p.match( nodelist )
    basename = match.group()

    p = re.compile( '\[[0-9,\-]+\]' )
    match = p.search( nodelist )

    p = re.compile( '[0-9,\-]+' )
    match = p.search( match.group( ))
    list = match.group()

    p = re.compile( '-' )
    nodes = []
    for i in list.split( ',' ):
        match = p.search( i )
        if match:
            tuple = i.split( '-' )
            for node in range( int( tuple[0] ), int( tuple[1] ) + 1 ):
                nodes.append( '%(name)s%(num)02d' % { 'name' : basename,
                                                      'num' :  node } )
        else:
            nodes.append( basename + i )
    return nodes

def launch( command ):
    os.popen( command )


nodes = getNodelist()
arguments = sys.argv
arguments[0] = 'gpu_sd'
command = ' '.join( arguments )

threads = []
for node in nodes:
    launchCmd = 'ssh ' + node + ' ' + command
    thread = threading.Thread( target=launch, args=(launchCmd,) )
    thread.start()
    threads.append( thread )

for thread in threads:
    thread.join()

