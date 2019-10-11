# -*- coding: utf-8 -*-
import datetime
import time
import sys
import socket
import json
'''
    Json format:
        "Timestamp" : time.time()
        "CmdType" : "Append" / "Connect"
        "Data" : string
'''

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

def sendAppendCmd(s):
    cmd = {"CmdType" : "Append", "Data" : s, "Timestamp" : time.time()}
    cmd = json.dumps(cmd).encode('utf-8')
    sock.sendto(cmd, (host_ip, HOST_UDP_PORT))

def sendConnectCmd(ip_addr):
    cmd = {"CmdType" : "Connect", "Data" : ip_addr, "Timestamp" : time.time()}
    cmd = json.dumps(cmd).encode('utf-8')
    sock.sendto(cmd, (host_ip, HOST_UDP_PORT))   

msg_encoded = msg.encode('utf-8')

sendConnectCmd(IPAddr)

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    if(data is None):
        continue
    msg = data.decode('utf8')
    print ("received message:", msg)
