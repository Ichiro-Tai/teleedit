# -*- coding: utf-8 -*-
import datetime
import time
import sys
import socket
import json

'''
    Json format:
        "CmdType" : "Append" / "Connect"
        "Timestamp" : time.time()
        "Data" : string
'''
HOST_TCP_PORT = 5005
CLIENT_TCP_PORT = 6006

if (len(sys.argv) - 1 < 1):
    print("Invalid format")
    exit(0)

host_ip = sys.argv[1]

hostname = socket.gethostname()
IPAddr = socket.gethostbyname(hostname)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_STREAM) # TCP
sock.connect((IPAddr, CLIENT_TCP_PORT))

def sendAppendCmd(s):
    # s is the string to append
    cmd = {"CmdType" : "Append", "Data" : s, "Timestamp" : time.time()}
    cmd = json.dumps(cmd).encode('utf-8')
    sock.send(cmd)

def sendConnectCmd(ip_addr):
    cmd = {"CmdType" : "Connect", "Data" : ip_addr, "Timestamp" : time.time()}
    cmd = json.dumps(cmd).encode('utf-8')
    sock.send(cmd)

cnt = 0

while True:
    if (cnt % 10 ==0):
        sendConnectCmd(IPAddr)
    cnt += 1
    data, addr = sock.recv(1024) # buffer size is 1024 bytes
    if(data is None):
        continue
    msg = data.decode('utf8')
    if (msg == "you are connected"):
        print ("Connected", msg)
        break

while True:
    print("Append input:")
    append_str = input()
    sendAppendCmd(append_str)
