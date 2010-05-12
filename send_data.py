#!/usr/bin/env python

import socket
import time
import sys
import random

infile = open(sys.argv[1])

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
serversocket.bind(('localhost', 31414))
serversocket.listen(5)

while True:
    print ('Waiting for connection')
    (clientsocket, address) = serversocket.accept()
    while True:
        #time.sleep(.25)
        time.sleep(0.0004)
        data = infile.readline()
        #data = infile.read(160) #int(random.random() * 1000))
        #print ('Sending: ',len(data)) #"%s"' % (data,) )
        try:
            clientsocket.send(data)
        except socket.error:
            print ('disconnected')
            break
