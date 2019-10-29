import datetime
import time
import sys
import socket
import json

HOST_TCP_PORT = 5005

class Client:
    def __init__(self, host_ip):
        self.client_ip = socket.gethostbyname(socket.gethostname())
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.host_ip = host_ip

        self.sock.connect((self.host_ip, HOST_TCP_PORT))

        self.send_connect_command()
        print(self.listen())
        

    def send_json_message(self, msg_dict):
        cmd_json = json.dumps(msg_dict).encode('utf-8')
        self.sock.send(cmd_json)

    def send_get_dir_command(self, path = ''):
        cmd = {
            'type' : 'get_dir',
            'data' : path
        }
        self.send_json_message(cmd) 

    def send_access_command(self, path):
        cmd = {
            'type' : 'access',
            'data' : path
        }
        self.send_json_message(cmd)

    def send_append_command(self, text):
        cmd = {
            'type' : 'append',
            'data' : text
        }
        self.send_json_message(cmd)

    def send_connect_command(self):
        cmd = {
            'type': 'connect'
        }
        self.send_json_message(cmd)

    def send_logoff_command(self):
        cmd = {
            'type': 'log_off'
        }
        self.send_json_message(cmd)

    def listen(self):
        while True:
            response = self.sock.recv(1024)
            msg = response.decode('utf8')
            if(msg is None):
                time.sleep(1)
            else:
                return msg



if __name__ == '__main__':
    from util import parse_start_input

    host_ip = parse_start_input()
    client = Client(host_ip)
    client.send_connect_command()
    client.send_access_command("t.txt")
    while True:
        print('Append text: ', end='')
        cmd = input()
        client.send_append_command(cmd)
        print(client.listen())
