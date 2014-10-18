#!/usr/bin/env python

import os
bashrc = os.getenv("HOME")+"/.profile"
cwd = os.getcwd()
#print bashrc
lines = []
try:
    bashrc_h = open(bashrc, "r")
    for line in bashrc_h:  
        lines.append(line)     
    bashrc_h.close()
    i = 0
    for l in lines:
        tmp = l.split(" ")
        if (tmp[0] == "export"):
            if (tmp[1].find("WORKFOCUS") != -1):
                print "Found \"export WORKFOCUS=\" on line #%d of %s" % (i,bashrc)
                lines[i] = "export WORKFOCUS="+cwd+"\n"
                print "Changed WORKFOCUS to %s" % (cwd)
                os.putenv("WORKFOCUS", cwd)
                bashrc_h = open(bashrc, "w")
                for l in lines:
                    bashrc_h.write(l)
                bashrc_h.close()
            else:
                print "Found \"export\" but no \"WORKFOCUS\" on line #%d." % (i)
        i += 1
except IOError, (errno, strerror):
    print "\nSomething got fucked up in the file IO: %s %s" % (errno, strerror),
