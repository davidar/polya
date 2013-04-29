struct corpus {
    hash_map<string,word_t> dict;
    vector<string> vocab;
    vector<word_t> text;
    vector<int> word_len;

    void init(void) {
#if !SPARSE_HASH
        dict.set_empty_key("");
#endif
        vocab.push_back("NULL");
    }

    void read(FILE *fin) {
        char buf[100];
        while(fscanf(fin, "%s", buf) != EOF)
            push_back(buf);
    }

    corpus() {
        init();
    }

    corpus(FILE *fin) {
        init();
        read(fin);
    }

    corpus(const char *fname) {
        init();
        char buf[100] = {};
        strcat(strcat(strcat(buf, "../data/"), fname), ".txt");
        FILE *fin = fopen(buf, "r");
        read(fin);
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

    const char *name(word_t w) {
        return vocab[w].c_str();
    }

    const char *get(int i) {
        return name(text[i]);
    }

    void print(int start, int n) {
        for(int i = start; i < start + n; i++)
            printf("%s ", get(i));
        printf("...\n");
    }
};
