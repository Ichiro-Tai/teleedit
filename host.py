# -*- coding: utf-8 -*-
import socket
import json
from _thread import *
import threading
import os
import queue
import util

HOST_TCP_PORT = 5005

global map_file_to_queue
map_file_to_queue = {}
class User:
    socket

'''

'''

def handle_file_thread(path):
    file = util.File(path)
    map_file_to_queue[path] = queue.Queue(0)
    while True:
        file.append(map_file_to_queue[path].get())
        file.save()
    pass

def listen_client_thread(client_sock, addr):
    file_path = ""
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
        elif msg['type'] == "log_off":
            break
        elif msg['type'] == "append":
            if file_path == "":
                print("File not specified")
            elif file_path in map_file_to_queue:
                map_file_to_queue[file_path].put(msg['data'])
        elif msg['type'] == "access":
            file_path = msg['data']
            if file_path in map_file_to_queue:
                pass
            else:
                start_new_thread(handle_file_thread, (file_path,))
        elif msg['type'] == "list_dir":
            s = json.dumps(os.listdir(msg['data'])).encode('utf-8')
            client_sock.send(s)
            pass
    client_sock.close()




host_name = socket.gethostname()
ip_addr = socket.gethostbyname(host_name)
print("Host Name:" + host_name)
print("Host IP addr:" + ip_addr)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_STREAM) # TCP
sock.bind((ip_addr, HOST_TCP_PORT))
sock.listen(5)
print("Listening:")
while True:
    client_socket, addr = sock.accept()
    print("New client\uff1a ", addr[0], addr[1])
    start_new_thread(listen_client_thread,
                    (client_socket, addr))

sock.close()
