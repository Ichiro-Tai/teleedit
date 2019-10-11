class File:
    f
    def __init__(self, s):
        self.f = open(s, "a+")
    def append(self, s):
        self.f.write(s)
    def close(self):
        self.f.close()
