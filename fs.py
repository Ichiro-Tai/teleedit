import json
import ast
from time import time
from stat import S_IFDIR, S_IFREG
from errno import ENOENT

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
        cmd = {
            'type': 'getattr',
            'path': path
        }
        self.client.send_json_message(cmd)
        attr_str = self.client.listen()
        attr = dict(x.split(':') for x in attr_str.split(';')[:-1])
        res = dict((k, int(attr[k])) for k in attr if attr[k])
        print(res)
        if not res:
            raise FuseOSError(ENOENT)
        return res

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
        return retval

    #--------------------------
    # File methods
    #--------------------------
    def create(self, path, mode):
        cmd = {
            'type': 'create',
            'path': path,
            'mode': mode
        }
        self.client.send_json_message(cmd)
        return 0

    def chown(self, path, uid, gid):
        cmd = {
            'type': 'chown',
            'path': path,
            'uid': uid,
            'gid': gid
        }
        self.client.send_json_message(cmd)

    def chmod(self, path, mode):
        cmd = {
            'type': 'chmod',
            'path': path,
            'mode': mode
        }
        self.client.send_json_message(cmd)

    def truncate(self, path, length, fh=None):
        cmd = {
            'type': 'truncate',
            'path': path,
            'length': length
        }
        self.client.send_json_message(cmd)

    def read(self, path, size, offset, fh):
        cmd = {
            'type': 'read',
            'path': path,
            'size': size,
            'offset': offset
        }
        self.client.send_json_message(cmd)
        buf = self.client.listen(size).encode('utf-8')
        return buf

    def write(self, path, data, offset, fh):
        cmd = {
            'type': 'write',
            'path': path,
            'data': data.decode('utf-8'),
            'offset': offset
        }
        self.client.send_json_message(cmd)
        return int(self.client.listen())

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
