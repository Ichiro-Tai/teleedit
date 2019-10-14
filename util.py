import time
SAVE_TIME_INTERVAL = 3
class File:
    f = ''
    fileName = ''
    lastSaved = 0

    def __init__(self, s):
        self.f = open(s, "a+")
        self.fileName = s
        self.lastSaved = time.time()

    def append(self, s):
        self.f.write(s)

    def save(self):
        now = time.time()
        if(now - self.lastSaved > SAVE_TIME_INTERVAL):
            self.lastSaved = now
            self.f.close()
            self.f = open(self.fileName, "a+")

    def close(self):
        self.f.close()
