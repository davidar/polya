# http://www.iro.umontreal.ca/~bengioy/apnews/
from array import array
vocab = map(str.strip, open('ap96ff.vocab','r').readlines()[4:])
words = array('L'); words.fromfile(open('ap96ff.btext','rb'), 13994528)
for w in words: print vocab[w]
