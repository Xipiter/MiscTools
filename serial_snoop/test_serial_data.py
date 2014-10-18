#!/usr/bin/env python
import sys
import time
global F_H

if __name__ == "__main__":
    argv = sys.argv
    if len(argv) != 2:
      print "usage: %s serialdev1 " % (sys.argv[0])
      exit(1)
    print "Press Ctrl-C to stop sending test data..."
    try:
        for i in range(0xff):
            F_H = open(argv[1],'wb') 
            F_H.write(chr(i))
            F_H.flush()
            F_H.close()
            print "Sent: ", repr(chr(i)),
            raw_input("Press Enter to send next byte")

    except KeyboardInterrupt:
        F_H.close()
        sys.exit(1) 
