import json
import ast
from time import time
from stat import S_IFDIR, S_IFREG

from fuse import FUSE, FuseOSError, Operations, LoggingMixIn

from client import Client

class FS(LoggingMixIn, Operations):
    def __init__(self, host_ip):
        self.client = Client(host_ip)
        self.fd = 0

    def destroy(self, path):
        self.client.send_disconnect_command()

    #--------------------------
    # Filesystem methods
    #--------------------------
    def getattr(self, path, fh):
        if path.startswith('/'):
            path = path[1:]

        cmd = {
            'type': 'getattr',
            'path': path
        }
        self.client.send_json_message(cmd)
        attr_str = self.client.listen()
        return ast.literal_eval(attr_str)

    def readdir(self, path, fh):
        cmd = {
            'type' : 'readdir',
            'path' : path
        }
        self.client.send_json_message(cmd)
        retval = self.client.listen()
        if retval.startswith('CANNOT OPEN DIR') or retval.startswith('EMPTY'):
            return []
        retval = retval.split('\n')[:-1]
        # print(retval)
        return retval

    #--------------------------
    # File methods
    #--------------------------
    def read(self, path, size, offset, fh):
        cmd = {
            'type': 'read',
            'path': path,
            'size': size,
            'offset': offset
        }
        client.send_json_message(cmd)
        return client.listen(size + 50) # 50 for potential overhead

    def write(self, path, data, offset, fh):
        cmd = {
            'type': 'write',
            'path': path,
            'data': data,
            'offset': offset
        }
        client.send_json_message(cmd)
        return client.listen()

if __name__ == '__main__':
    import tempfile
    from util import parse_start_input

    host_ip = parse_start_input()

    with tempfile.TemporaryDirectory() as directory:
        print('Mountpoint at:', directory)
        fuse = FUSE(
            FS(host_ip),
            directory,
            foreground=True,
            nothreads=True
        )
