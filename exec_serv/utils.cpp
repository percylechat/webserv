int count_occ(std::string txt, std::string word){
// count number of word occurences in string
    int j = 0;
    while (1){
        std::size_t i = txt.find(word);
        if (i == txt.npos)
            return j;
        j++;
        txt = txt.substr(i + word.length(), txt.length() - (i + word.length()));
    }
}