CXX = g++-4.7
CXXFLAGS = -std=c++11 -O3 -DDENSEHASH

hpylm: CRP.cc hpylm.cc *.h
	$(CXX) $(CXXFLAGS) -o $@ CRP.cc hpylm.cc

hpylm-apnews: hpylm
	gunzip < ../data/apnews.txt.gz | ./hpylm 3 963071 300 125

hpylm-brown: hpylm
	./hpylm 3 100000 10 1 < ../data/brown.full.txt
