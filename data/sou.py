from nltk.corpus import state_union

test  = [fid for fid in state_union.fileids() if 'Johnson' in fid]
train = [fid for fid in state_union.fileids() if fid not in test]

print 'TEST:', ', '.join(test)
f = open('sou.test.txt','w')
for w in state_union.words(test): print>>f, w
f.close()

print 'TRAIN:', ', '.join(train)
f = open('sou.train.txt','w')
for w in state_union.words(train): print>>f, w
f.close()
