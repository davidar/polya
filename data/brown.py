import nltk
from nltk.corpus import brown

freq = {}
for w in brown.words():
    freq[w] = freq.get(w,0) + 1

f = open('brown.reduc.txt','w')
for w in brown.words():
    if freq[w] <= 3: w = 'RARE_WORD'
    print>>f, w
f.close()

f = open('brown.tagged.txt','w')
for w,t in brown.tagged_words():
    print>>f, w, t
f.close()

f = open('brown.tagged.simp.txt','w')
for w,t in brown.tagged_words(simplify_tags=True):
    if t.strip() == '': t = w
    print>>f, w, t
f.close()

f = open('brown.full.txt','w')
for w in brown.words():
    print>>f, w
f.close()

f = open('brown.norm.txt','w')
for s in brown.sents():
	s = ' '.join(s).lower()
	s = s.replace('--','&ndash;').replace('-',' - ').replace('&ndash;','--')
	s = s.replace('? ?','?').replace('; ;',';')
	s = ' '.join(nltk.word_tokenize(s))
	print>>f, s
f.close()
