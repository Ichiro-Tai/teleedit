# -*- coding: utf-8 -*-

import socket
import json
from util import File


HOST_UDP_PORT = 5005
CLIENT_UDP_PORT = 6006
test_file = File('test.txt')

hostname = socket.gethostname()
ip_addr = socket.gethostbyname(hostname)

print("Your Computer Name is:" + hostname)
print("Your Computer IP Address is:" + ip_addr)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((ip_addr, HOST_UDP_PORT))

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    if(data is None):
        continue
    msg = data.decode('utf8')
    print ("received message:", msg)

    msg = json.loads(msg)

    # Receive Connect Cmd
    if msg['CmdType'] == "Connect":
        client = msg['Data']
        greetings = "you are connected"
        greetings_encoded = greetings.encode("utf-8")
        sock.sendto(greetings_encoded, (client, CLIENT_UDP_PORT))
    
    if msg['CmdType'] == "Append":
        test_file.append(msg['Data'])
        test_file.save()
    
