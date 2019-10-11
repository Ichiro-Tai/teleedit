# -*- coding: utf-8 -*-

import sys
import socket

if (len(sys.argv) - 1 < 1):
    print("Invalid format")
    exit(0)

host_ip = sys.argv[1]

UDP_PORT = 5005

msg = "TEST MESSAGE FROM CLIENT"

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP

msg_encoded = msg.encode('utf-8')

sock.sendto(msg_encoded, (host_ip, UDP_PORT))
