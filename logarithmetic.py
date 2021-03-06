import math

def log(x, base=None):
    if type(x) is not LogR and x == 0: return float('-inf')
    if base is None:
        return x.log if type(x) is LogR else math.log(x)
    else:
        return x.log / math.log(base) if type(x) is LogR else \
               math.log(x, base)

class LogR(object):
    def __init__(self, x=0, isLog=False):
        if isLog:
            self.log = x
        else:
            self.log = log(x)
    def __float__(self):
        return math.exp(self.log)
    
    def __mul__(x,y):
        return LogR.exp(log(x) + log(y))
    def __div__(x,y):
        return LogR.exp(log(x) - log(y))
    
    def __add__(x,y):
        # TODO numpy.logaddexp
        return x * (1 + float(y / x)) if x >= y else y + x
    def __sub__(x,y):
        if x >= y:
            return x * (1 - float(y / x))
        else:
            raise ValueError(
                'cannot subtract ' + str(y) + ' from ' + str(x))
    
    def __cmp__(x,y):
        return cmp(log(x), log(y))
    
    @staticmethod
    def exp(x):
        return LogR(x, isLog=True)
    
    @staticmethod
    def pow(x,y):
        return LogR.exp(log(x) * y)
    def __pow__(x,y):
        return LogR.pow(x,y)
    
    def __str__(self):
        frac, exponent = math.modf(log(self, 10))
        coeff = 10**frac
        if coeff < 1:
            coeff *= 10
            exponent -= 1
        return str(coeff) + "e%+d" % int(exponent)
    
    
    @staticmethod
    def gamma(x):
        return LogR.exp(math.lgamma(x))
    
    @staticmethod
    def poch(x,n,k=1):
        '''Rising factorial / k-Pochhammer symbol'''
        if   k == 0:
            return LogR.pow(x,n)
        elif k == 1:
            return LogR.gamma(x + n) / LogR.gamma(x) if n > 0 else LogR(1)
        else:
            return LogR.pow(k,n) * LogR.poch(float(x)/k,n)
    
    @staticmethod
    def product(seq, init=1):
        return LogR.exp(sum(map(log,seq),log(init)))
