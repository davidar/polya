CXX = g++-4.7 -std=c++11
CXXFLAGS = -Ofast -DDENSEHASH -DSLICESAMP

hpylm: CRP.cc hpylm.cc *.h
	$(CXX) $(CXXFLAGS) -o $@ CRP.cc hpylm.cc

test: CRP.cc test.cc *.h
	$(CXX) $(CXXFLAGS) -o $@ CRP.cc test.cc && ./test

hpylm-apnews: hpylm
	gunzip < ../data/apnews.txt.gz | ./hpylm 3 963071 300 125

hpylm-brown: hpylm
	./hpylm 3 100000 10 1 < ../data/brown.full.txt
