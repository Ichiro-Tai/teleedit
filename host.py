# -*- coding: utf-8 -*-
import socket
import json
from _thread import *
import threading

HOST_TCP_PORT = 5005

def listen_client_thread(client_sock, addr):
    while True:
        data = client_sock.recv(1024) # buffer size is 1024 bytes
        if(data is None):
            continue
        msg = data.decode('utf8')
        print ("received message:", msg)
        msg = json.loads(msg)
        # Receive Connect Cmd
        if msg['type'] == "connect":
            greetings = "you are connected"
            greetings_encoded = greetings.encode("utf-8")
            client_sock.send(greetings_encoded)
    client_sock.close()

host_name = socket.gethostname()
ip_addr = socket.gethostbyname(host_name)
print("Your Computer Name is:" + host_name)
print("Your Computer IP Address is:" + ip_addr)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_STREAM) # TCP
sock.bind((ip_addr, HOST_TCP_PORT))
sock.listen(5)
print("Listening:")
while True:
    client_socket, addr = sock.accept()
    print("New client\uff1a ", client_socket, addr[0], addr[1])
    start_new_thread(listen_client_thread,
                    (client_socket, addr))

sock.close()
