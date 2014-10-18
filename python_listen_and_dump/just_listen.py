#!/usr/bin/env python
#
# Just something to listen on a port and hexdump out whatever it gets.
#
#

import socket
import sys

class Listener:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port

    def listening_server(self):
        self.connectback_h = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connectback_h.bind((self.ip, self.port))
        self.connectback_h.listen(5)
        while 1:
            (clientsocket, address) = self.connectback_h.accept() 
            self.hsend = clientsocket.send
            self.hrecv = clientsocket.recv
            print "Connect from ",  address, " ." 
            while 1:
                data = clientsocket.recv(1024)
                print self.hexdump(data)
                if not data: break
        clientsocket.close() 

    def hexdump(self, src, length=16):
        N=0; result=''
        self.FILTER=''.join([(len(repr(chr(x)))==3) and chr(x) or '.' for x in range(256)])
        while src:
            s,src = src[:length],src[length:]
            hexa = ' '.join(["%02X"%ord(x) for x in s]) 
            s = s.translate(self.FILTER)
            result += "%08X:  %-*s  |%s|\n" % (N, length*3, hexa, s)
            N+=length
        return result

if __name__=="__main__":
    import optparse
    parser = optparse.OptionParser()
    parser.add_option(
        '-l','--local-ip',
        dest='local_ip',default='127.0.0.1',
        help='IP of interface to bind to')
    parser.add_option(
        '-p','--local-port',
        type='int',dest='local_port',default=80,
        help='Port to bind to')
    if len(sys.argv) == 1:
        sys.argv.append("--help")
    options, args = parser.parse_args()
    print "Listening on %s:%d..." % (options.local_ip, options.local_port)
    try:
        serv = Listener(options.local_ip, options.local_port)
        serv.listening_server() 
    except KeyboardInterrupt:
        print "Quitting..."
        sys.exit(0)
