# -*- coding: utf-8 -*-
import datetime
import time
import sys
import socket

if (len(sys.argv) - 1 < 1):
    print("Invalid format")
    exit(0)

host_ip = sys.argv[1]

HOST_UDP_PORT = 5005
CLIENT_UDP_PORT = 6006

hostname = socket.gethostname()
IPAddr = socket.gethostbyname(hostname)

connect_msg = "C"


msg = str(time.time())
msg += " " + connect_msg + " " + IPAddr

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((IPAddr, CLIENT_UDP_PORT))

msg_encoded = msg.encode('utf-8')

sock.sendto(msg_encoded, (host_ip, HOST_UDP_PORT))

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    if(data is None):
        continue
    msg = data.decode('utf8')
    print ("received message:", msg)
