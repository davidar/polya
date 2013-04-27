struct corpus {
    hash_map<string,word_t> dict;
    vector<string> vocab;
    vector<word_t> text;
    vector<int> word_len;

    corpus() {
#if !SPARSE_HASH
        dict.set_empty_key("");
#endif
        vocab.push_back("NULL");
    }

    void push_back(word_t w) {
        text.push_back(w);
    }

    void push_back(string s) {
        if(!dict.count(s)) { // new word
            word_t w = dict.size() + 1;
            dict[s] = w;
            vocab.push_back(s);
            assert(vocab[w] == s);
        }
        word_len.push_back(s.length());
        push_back(dict[s]);
    }

    void push_back(char *buf) {
        push_back(string(buf));
    }

    int size(void) {
        return text.size();
    }

    word_t &operator[](int i) {
        return text[i];
    }
};
