#!/usr/bin/env python2.4
#Find and remove all "^M" control-M newlines.
"""

REMOVE FAT32/16 newlines.

    This will remove all ^M newlines from a file or recursively from all
files within a directory tree. It will skip files it thinks are binary (i.e. 
containing more than 40% of non-printable characters within the first 2048 bytes
of the file).

USAGE:
  ./ctrlM_clean.py <file or directory>

"""

import os, sys, getopt

def isAscii(fname):
    """

        1. get a file name
        2. read the first 2048 bytes of file
        3. count the number of non-printable characters
        4. if below threshold decide if file is binary or ascii
        5. return true if ascii      
    
    """
    sample_size = 2048.00
    percent = 0
    np_count = 0 #counter for number of non printable characters
    try:
        f_h = open(fname, 'r')
    except IOError, msg:
        print "\nAn error occurred while opening %s" % (fname)
        sys.exit(1)
    sample = f_h.read(2048)
    sample_size = len(sample)
    f_h.close()
    for byte in sample:
        if not ((ord(byte) > 0) and (ord(byte) < 127)):
            np_count+=1
    percent = int(((np_count/float(sample_size)) * 100))
    #print ("%s contains %d/%d (%d percent) non-printable characters.") % (fname, np_count, sample_size, percent)
    if percent >= 40:
        return False
    else:
        return True

def clean_file(fname):
    """ 
        1. get a file name.
        2. open the file.
        3. check if the file is mostly binary or mostly text.
        4. remove ^Ms from text files.
    """
    try:
        f_h = open(fname, 'r')
    except IOError, msg:
        print "\nAn error occurred while opening %s" % (fname)
        sys.exit(1)
    f_bytes = f_h.read()
    f_h.close()
    count = f_bytes.count("\x0d\x0a")
    if count > 0:
        f_bytes = f_bytes.replace("\x0d\x0a", "\x0a")
        try:
            f_h = open(fname, 'wb')
            f_h.write(f_bytes)
            f_h.close()
            print ("%s contained %d ^Ms which were cleansed.") % (fname, count)
        except IOError, msg:
            print "\nAn error occurred while trying to write %s" % (fname)
    else:
        print ("%s contained no ^Ms...skipping.") % (fname) 

def start_cleaning(fname):
    """
        1. get filename
        2. check if file or dir.
        3. if file, check if binary or ascii
        4. if ascii, clean
    """
    def _cleaner(fname):
        if isAscii(fname):
            clean_file(fname)
        else:
            print "%s is probably binary, skipping..." % (fname)

    if os.path.isdir(fname):
        print "%s is a directory. Recursing into it." % (fname)
        for root, dirs, files in os.walk(fname, topdown=False):
            for name in files:
                _cleaner(os.path.join(root, name))
    elif os.path.isfile(fname):
        _cleaner(fname)

    elif os.path.islink(fname):
        print "Skipping symlink: %s" % (fname)
        
def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:],\
            "h", ["help"])
    except getopt.error, msg:
        print msg
        print __doc__
        sys.exit(2)
    if len(sys.argv) <= 1:
        print __doc__
        sys.exit(1)
    for o, a in opts:
        if o in ("-h", "--help"):
            print __doc__
            sys.exit(0)
    start_cleaning(sys.argv[1])

if __name__ == "__main__":
    main()

