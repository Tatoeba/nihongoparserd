#ifndef NIHONGOPARSERD_FURIGANA_H
#define NIHONGOPARSERD_FURIGANA_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

class Furigana {
  private:
    std::unordered_map<uint32_t, uint32_t> KataToHiraTable;

  public:
    Furigana();
    std::string katakana_to_hiragana(std::string katakana);
    std::vector<std::pair<std::string, std::string> > tokenize(std::string kanji, std::string reading);
};

#endif
