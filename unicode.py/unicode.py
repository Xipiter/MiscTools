#!/usr/bin/env python
"""
UNICODE.PY

Encode and decode unicode files.
    -d <filename> || --decode=<filename> : Filename to be uni-decoded
    -e <filename> || --encode=<filename> : Filename to be uni-encoded
    -b <filename> || --showbom=<filename>: Check the encoding of a unicode file
    -s <string> || --string-encode=<string>: UTF-16 encode <string>
    -r <string> || --string-decode=<string>: Decode a UTF-16 encoded <string>

--sa7  
"""

import sys
import getopt
import os
from UserDict import UserDict
import string

class TwoWayHash(UserDict):
    def __init__(self, dict):
        UserDict.__init__(self,dict)
        for (key,val) in self.items():
            self[val] = key

BOMTable = TwoWayHash( {
    "UTF-32 BIG Endian" : "\x00\x00\xFE\xFF",
    "UTF-32 LITTLE Endian" : "\xFF\xFE\x00\x00",
    "UTF-16 BIG Endian" : "\xFE\xFF",
    "UTF-16 LITTLE Endian" : "\xFF\xFE",
    "UTF-8" : "\xEF\xBB\xBF"
})

def decode_unicode(file):
    show_bom(file)        
    ext_str = '.uni-decoded'
    out_file_name = file+ext_str
    bytes = []
    bytes_str = ""
    i=0
    try:
        in_file = open(file, "rb") #binary mode.
        bytes = in_file.read()
        print "\nRead %d bytes from %s " % (len(bytes),file),
        out_file = open(out_file_name,"wb")
        in_file.close()
    except IOError, (errno, strerror): 
        print "\nYou fucked something up!I/O error(%s): %s" % (errno, strerror),
    for a in range(len(bytes)/2):
        i=a*2
        if i in [0,1]: #skip the first two
            continue
        tmp = bytes[i:i+2]
        bytes_str += unicode(tmp.decode('utf-16').__str__(), 'utf-8')

    try:
        out_file.write(bytes_str)
        print "\nWrote %d bytes to %s" % ((i/2),out_file_name),
        out_file.close()
    except IOError, (errno, strerror):
        print "\nYou fucked something up!I/O error(%s): %s" % (errno, strerror)

def encode_unicode(file):
    ext_str = '.uni-encoded'
    bom_val = ""
    out_file_name = file+ext_str
    bytes = []
    bytes_str = ""
    try:
        in_file = open(file, "rb") #binary mode.
        bytes = in_file.read()
        in_file.close()
        print "\nRead %d bytes from %s" % (len(bytes),file),
        out_file = open(out_file_name,"wb")
        join = str.join #grab the function for ease of use.
        bytes_str = join('', bytes)
        bytes_str = unicode(bytes_str,'utf-8') #just to be suuuuuure *shifty eyes*
        bytes_str = bytes_str.encode('utf-16') #jacked from Kris Kendal
        out_file.write(bytes_str)
        print "\nWrote %d bytes to %s" % (len(bytes_str),out_file_name),
        out_file.close()
    except IOError, (errno, strerror):
        print "\nYou fucked something up!I/O error(%s): %s" % (errno, strerror),

def show_bom(file):
    try:
        in_file = open(file, "rb") #binary mode.
        two_bytes = []
        four_bytes = []
        two_bytes = in_file.read(2)
        in_file.seek(0) # point file pointer back at the beginning 
        four_bytes = in_file.read(4)
        join = str.join
        two_bytes_str = join('', two_bytes)
        four_bytes_str = join('', four_bytes) 
        if four_bytes_str in BOMTable:
            encoding = BOMTable[four_bytes_str]
            print "\nThat file prolly has \"%s\" encoding" % (encoding)
        else:
            if four_bytes_str[:3] in BOMTable:
                encoding = BOMTable[four_bytes_str[:3]]
                print "\nThat file prolly has \"%s\" encoding" % (encoding)
            else:
                if two_bytes_str in BOMTable:
                    encoding = BOMTable[two_bytes_str]
                    print "\nThat file prolly has \"%s\" encoding" % (encoding)
                else:
                    print "\nUnable to determine the UTF encoding of that file. 'You sure its Unicode?" 
    
    except IOError, (errno, strerror):
        print "\nYou fucked something up!I/O error(%s): %s" % (errno, strerror),
    
def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:e:b:s:r:", ["decode=","encode=","bom=","string-encode=", "string-decode="])
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
        if o in ("-e", "--encode"):
            encode_unicode(a)
        if o in ("-d", "--decode"):
            decode_unicode(a)
        if o in ("-b", "--bom"):
            show_bom(a)
        if o in ("-s", "--string-encode"): #jacked from Kris Kendall
            buf=eval('"%s"'%(a.replace('"','\\"'),))
            tmp = unicode(buf,'utf-8')
            b = tmp.encode('utf-16')[2:] #the "2:" skips over the BOM (Byte order Marking)
            hex_tmp = ''.join(["\\x%02X"%ord(x) for x in b])
            print hex_tmp
        if o in ("-r", "--string-decode"): #jacked from Kris Kendall
            buf=eval('"%s"'%(a.replace('"','\\"'),))
            tmp = unicode(buf,'utf-16')
            b = tmp.encode('utf-8')
            hex_tmp = ''.join(["\\x%02X"%ord(x) for x in b])
            print hex_tmp

if __name__ == "__main__":
    main()
