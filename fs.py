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
        path = path.encode('utf-8')
        cmd = 'getattr'.ljust(8) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        attr_str = self.client.listen()
        attr = dict(x.split(':') for x in attr_str.split(';')[:-1])
        res = dict((k, int(attr[k])) for k in attr if attr[k])
        print(res)
        if not res:
            raise FuseOSError(ENOENT)
        return res

    def readdir(self, path, fh):
        path = path.encode('utf-8')
        cmd = 'readdir'.ljust(8) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        retval = self.client.listen()
        if retval.startswith('CANNOT OPEN DIR') or retval.startswith('EMPTY'):
            return []
        retval = retval.split('\n')[:-1]
        return retval

    #--------------------------
    # File methods
    #--------------------------
    def create(self, path, mode):
        path = path.encode('utf-8')
        cmd = 'create'.ljust(8) + str(mode).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        return 0

    def chown(self, path, uid, gid):
        path = path.encode('utf-8')
        cmd = 'chown'.ljust(8) + str(uid).ljust(16) + str(gid).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)

    def chmod(self, path, mode):
        path = path.encode('utf-8')
        cmd = 'chmod'.ljust(8) + str(mode).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)

    def truncate(self, path, length, fh=None):
        path = path.encode('utf-8')
        cmd = 'truncate'.ljust(8) + str(length).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)

    def read(self, path, size, offset, fh):
        path = path.encode('utf-8')
        cmd = 'read'.ljust(8) + str(size).ljust(16) + str(offset).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        buf = self.client.listen(size).encode('utf-8')
        return buf

    def write(self, path, data, offset, fh):
        path = path.encode('utf-8')
        #data = data.decode('utf-8').encode('utf-8')
        cmd = 'write'.ljust(8) + str(offset).ljust(16) + str(len(path)).ljust(16) + str(len(data)).ljust(16) + path + data
        self.client.send_str_msg(cmd)
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
