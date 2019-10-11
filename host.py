# -*- coding: utf-8 -*-

import socket

clients = []

UDP_IP = "127.0.0.1"
UDP_PORT = 5005

hostname = socket.gethostname()
ip_addr = socket.gethostbyname(hostname)

print("Your Computer Name is:" + hostname)
print("Your Computer IP Address is:" + ip_addr)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((ip_addr, UDP_PORT))

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    if(data is None):
        continue
    print ("received message:", data.decode('utf8'))
