!/usr/bin/env python
"""
JAD ALL

    "Recursive" hack for JAD. Will execute JAD and dump the .java output by jad
    into the same directory as the .class

    USAGE:
        jadall.py -j <full path to jad executable> -d <full path to directory with java code> 

"""
import os
import sys
import getopt

global jadexe

def _checkdir(adir):
#    print "Recursing into directory %s" % (adir)
    for root, dirs, files in os.walk(adir, topdown=False):
        for file in files:
            if file.endswith(".class"):
                print "Decompiling ", root+os.path.sep+file
                cmd = "%s -r -o -s .java %s" % (jadexe, root+os.path.sep+file)
		#print "\t",cmd
                os.popen(cmd)
		
        for a in dirs:
            _checkdir(root+os.path.sep+a)
try: 
    opts, args = getopt.getopt(sys.argv[1:],\
        "-j:d:", ["--jad=", "--dir="])
except getopt.error:
    print __doc__
    sys.exit(2)
if len(sys.argv) <= 1:
    print __doc__
    sys.exit(2)
else:
    print __doc__

for o, a in opts:
    if o in ("-j", "--jad"):
        jadexe = a
    if o in ("-d", "--dir"):
        checkdir = a

if not os.path.exists(checkdir):
    print "Directory %s does not exist. Quitting" % checkdir
    sys.exit(1)
elif not os.path.exists(jadexe):
    print "Jad executable not found here: %s" % jadexe 
else:
    print("Checking the directory: %s" % checkdir)
    os.chdir(checkdir)
    _checkdir(checkdir)

