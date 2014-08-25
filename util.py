import operator

def prod(xs):
    return reduce(operator.mul, xs, 1)

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
    def __missing__(self, key):
        val = self[key] = self.f(key)
        return val
