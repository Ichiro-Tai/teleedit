import json
from fuse import FUSE, FuseOSError, Operations, LoggingMixIn

from client import Client


class FS(LoggingMixIn, Operations):
    def __init__(self, host_ip):
        self.client = Client(host_ip)

    def destroy(self, path):
        # self.client.close() # TODO: check correctness
        pass

    def read(self, path, size, offset, fh):
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
        cmd = {
            'type': 'write',
            'path': path,
            'data': data,
            'offset': offset
        }
        client.send_json_message(cmd)
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
