#!/usr/bin/python
import sys
if len(sys.argv) != 2:
    print "usage: %s <str>" % sys.argv[0]
    sys.exit(0)

buf=eval('"%s"'%(sys.argv[1].replace('"','\\"'),))

a = unicode(buf,'utf-16')
b = a.encode('utf-8')
hexa = ''.join(["\\x%02X"%ord(x) for x in b])
print hexa
