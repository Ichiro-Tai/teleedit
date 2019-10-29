import json
from time import time
from stat import S_IFDIR, S_IFREG

from fuse import FUSE, FuseOSError, Operations, LoggingMixIn

from client import Client

class FS(LoggingMixIn, Operations):
    def __init__(self, host_ip):
        # self.client = Client(host_ip)
        self.fd = 0

    def destroy(self, path):
        # self.client.close() # TODO: check correctness
        pass

    #--------------------------
    # Filesystem methods
    #--------------------------
    def getattr(self, path, fh):
        if path.startswith('/'):
            path = path[1:]

        print(path)
        if path == 'abc.txt':
            return dict(
                st_mode=(S_IFREG),
                st_nlink=3, # TODO: change this value
                st_size=100,
                st_ctime=0,
                st_mtime=0,
                st_atime=time()
            )
        else:
            return dict(
                st_mode=(S_IFDIR),
                st_nlink=3, # TODO: change this value
                st_size=100,
                st_ctime=0,
                st_mtime=0,
                st_atime=time()
            )

    def readdir(self, path, fh):
        return ['.', '..'] + ['abc.txt']

        # TODO: read dir from host

    #--------------------------
    # File methods
    #--------------------------
    def open(self, path, flags):
        self.fd += 1
        return self.fd

    def create(self, path, fh):
        return 0

    def read(self, path, size, offset, fh):
        print(fh)
        return 'xyz read abc'

        cmd = {
            'type': 'read',
            'path': path,
            'size': size,
            'offset': offset
        }
        client.send_json_message(cmd)
        read_buf = client.listen()
        return read_buf

    def write(self, path, data, offset, fh):
        return 100

        cmd = {
            'type': 'write',
            'path': path,
            'data': data,
            'offset': offset
        }
        client.send_json_message(cmd)
        # TODO: get feedback from host
        return len(data)

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
