import datetime
import time
import sys
import socket

HOST_TCP_PORT = 5005

class Client:
    def __init__(self, host_ip):
        self.client_ip = socket.gethostbyname(socket.gethostname())
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.host_ip = host_ip

        self.sock.connect((self.host_ip, HOST_TCP_PORT))

        self.send_connect_command()
        print(self.listen())

    def send_str_msg(self, msg_str):
        self.sock.send(msg_str.encode())

    def send_byte_msg(self, msg):
        self.sock.send(msg)

    def send_append_command(self, text):
        cmd = 'append'.ljust(8)
        self.send_str_msg(cmd)

    def send_connect_command(self):
        cmd = 'connect'.ljust(8)
        self.send_str_msg(cmd)

    def send_disconnect_command(self):
        cmd = 'dconnect'.ljust(8)
        self.send_str_msg(cmd)

    def listen(self, size=1024):
        while True:
            response = self.sock.recv(size)
            msg = response.decode('utf8')
            if(msg is None):
                time.sleep(1)
            else:
                return msg



if __name__ == '__main__':
    from util import parse_start_input

    host_ip = parse_start_input()
    client = Client(host_ip)
    client.send_append_command("HI")
    '''
    client.send_connect_command()
    client.send_open_command("t.txt")
    while True:
        print('Append text: ', end='')
        cmd = input()
        client.send_append_command(cmd)
        print(client.listen())'''
