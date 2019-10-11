# -*- coding: utf-8 -*-

import socket
from util import File

clients = []

UDP_PORT = 5005
test_file = File('test.txt')

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
    msg = data.decode('utf8')
    print ("received message:", msg.decode('utf8'))

    message_array = str.split(" ", 3)
    timestamp = message_array[0]
    command_type = message_array[1]
    message = message_array[2]

    #ip address
    if command_type == "C":


    test_file.append(msg)
