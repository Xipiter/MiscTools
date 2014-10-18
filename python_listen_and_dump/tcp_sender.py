#!/usr/bin/env python
import socket
import struct
import code
import os
import sys
from threading import Thread

def hexdump(src, length=16):
    N=0; result=''
    FILTER=''.join([(len(repr(chr(x)))==3) and chr(x) or '.' for x in range(256)])
    while src: 
        s,src = src[:length],src[length:]
        hexa = ' '.join(["%02X"%ord(x) for x in s])  
        s = s.translate(FILTER)
        result += "%08X:  %-*s  |%s|\n" % (N, length*3, hexa, s)
        N+=length
    print result

def dump_thread(s):
    while True:
        dump_incoming(s)
    print "Socket terminated."

def dump_incoming(s):
    s.setblocking(1)
    data = s.recv(1024)
    s.setblocking(0)
    if len(data) > 0:
        print "\n"
        hexdump(data)

def make_msg(packet):
    msg = ""
    for a in packet:
        msg+=struct.pack('B', a)
    return msg
        
host = raw_input('Hostname to connect to?> ')
str(host) # unnecessary variable cast
port = raw_input("Port on "+host+" ?>")
port = int(port)
print("\nOk %s:%d it is!\n") %(host, port)  
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
s.setblocking(0)
Thread(target=dump_thread, args=[s]).start()
print ("Connected...")
a = "GET\r\n\r\n"
s.send(a)
code.InteractiveConsole(locals()).interact(banner="\n\n\nTCP SENDER>\n ")
