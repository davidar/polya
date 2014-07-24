import nltk
from nltk.corpus import state_union

test  = [fid for fid in state_union.fileids() if 'Johnson' in fid]
train = [fid for fid in state_union.fileids() if fid not in test]

print 'TEST:', ', '.join(test)

f = open('sou.test.txt','w')
for w in state_union.words(test): print>>f, w
f.close()

f = open('sou.norm.test.txt','w')
for s in state_union.sents(test):
	s = ' '.join(s).lower()
	s = s.replace("' s ","'s ").replace(' .','.')
	s = ' '.join(nltk.word_tokenize(s))
	print>>f, s
f.close()

print 'TRAIN:', ', '.join(train)

f = open('sou.train.txt','w')
for w in state_union.words(train): print>>f, w
f.close()

f = open('sou.norm.train.txt','w')
for s in state_union.sents(train):
	s = ' '.join(s).lower()
	s = s.replace("' s ","'s ").replace(' .','.')
	s = ' '.join(nltk.word_tokenize(s))
	print>>f, s
f.close()
