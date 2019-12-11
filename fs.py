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
        cmd = 'getattr'.ljust(8) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        attr_str = self.client.listen()
        print("attr str:", attr_str)
        attr = dict(x.split(':') for x in attr_str.split(';')[:-1])
        res = dict((k, int(attr[k])) for k in attr if attr[k])
        print(res)
        if not res:
            raise FuseOSError(ENOENT)
        return res

    def readdir(self, path, fh):
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
        cmd = 'create'.ljust(8) + str(mode).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        return 0

    def mkdir(self, path, mode):
        cmd = 'mkdir'.ljust(8) + str(mode).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        return 0

    def rmdir(self, path):
        cmd = 'delete'.ljust(8) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        return 0

    def unlink(self, path):
        cmd = 'delete'.ljust(8) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        return 0

    def chown(self, path, uid, gid):
        cmd = 'chown'.ljust(8) + str(uid).ljust(16) + str(gid).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)

    def chmod(self, path, mode):
        cmd = 'chmod'.ljust(8) + str(mode).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)

    def truncate(self, path, length, fh=None):
        cmd = 'truncate'.ljust(8) + str(length).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)

    def read(self, path, size, offset, fh):
        cmd = 'read'.ljust(8) + str(size).ljust(16) + str(offset).ljust(16) + str(len(path)).ljust(16) + path
        self.client.send_str_msg(cmd)
        buf = self.client.listen(size)
        return buf

    def write(self, path, data, offset, fh):
        cmd = ('write'.ljust(8) + str(offset).ljust(16) + str(len(path)).ljust(16) + path + str(len(data)).ljust(16)).encode() + data
        self.client.send_byte_msg(cmd)
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
