#!/usr/bin/env python
import os
import slurm
import sys
import threading

nodes = slurm.getNodelist()
arguments = sys.argv
arguments[0] = 'gpu_sd'
command = ' '.join( arguments )

def launch( command ):
    os.exec( command )

threads = []
for node in nodes:
    launchCmd = 'ssh ' + node + ' ' + command
    thread = threading.Thread( target=launch, args=(launchCmd,) )
    thread.start()
    threads.append( thread )

for thread in threads:
    thread.join()

