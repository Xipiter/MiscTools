#!/usr/bin/env python
"""
UNUNICODE-IZE

Read in <file> starting at <offset>, write to <file>.ununicode skipping every other 
character. (which if unicode, will be the nulls)

    -f <filename> || --file=<filename> : File to be un-unicoded

OFFSET THING IS BROKE TILL I FIX GETOPT
    -o <number> || --offset=<number> : Start <number> bytes into the file


--sa7  
"""

import sys
import getopt
import os

def read_and_write(file, skip):
    in_file = open(file, "rb") #binary mode.
    out_file_name = file+'.ununicode'
    bytes = []
    i = 0
    in_file.seek(skip)
    bytes = in_file.read() #without a size read() goes to EOF, phat!
    count = len(bytes)
    print "\nRead %d bytes from %s offset %d " % (count,file,skip)
    out_file = open(out_file_name,"wb")
    while i < count:
        out_file.write(bytes[i]) 
        i += 2 #skip over nullio cuz I enjoy falafel
    print "\nWrote %d bytes to %s" % ((i/2),out_file_name)
    in_file.close()
    out_file.close()
    
def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "f:o:", ["file=","offset="])
    except getopt.error, msg:
        print msg
        print __doc__
        sys.exit(2)
    # process options
    if len(sys.argv) <= 1:
        print __doc__
        sys.exit(2)
    for o, a in opts:
        offset = 0 # is this even necessary!?
        if o in ("-o", "--offset"):
            offset = a
        if o in ("-f", "--file"):
            if offset > 0:
                read_and_write(a, offset)
            else:
                read_and_write(a, 2)

if __name__ == "__main__":
    main()
