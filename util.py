import math
import time
import datetime

pos = lambda x: x if x > 0 else 0
neg = lambda x: pos(-x)

class DynEnum(object):
    def __init__(self):
        self.ident = {} # str -> int
        self.names = [] # int -> str
    def __call__(self, name):
        if name in self.ident:
            return self.ident[name]
        else:
            i = len(self)
            self.ident[name] = i
            self.names.append(name)
            return i
    def __getitem__(self, i):
        return self.names[i]
    def __len__(self):
        return len(self.names)

class DynDict(dict):
    def __init__(self, f):
        dict.__init__(self)
        self.f = f
    def __getitem__(self, key):
        if key in self:
            return dict.__getitem__(self, key)
        else:
            val = self.f(key)
            self[key] = val
            return val

#class ETA(object):
#    def __init__(self):
#        self.start = time.clock()
#    def __call__(self, i, n=1.0):
#        s = (n - i) * (time.clock() - self.start) / i
#        return datetime.timedelta(seconds=int(s))
