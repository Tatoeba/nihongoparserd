
#include <cstring>
#include <algorithm>

#include "Furigana.h"
#include "Utf8.h"

#define WARIFURI_SEPARATOR '.'

static const char katakanas[][4] = {
    "ァ","ア","ィ","イ","ゥ","ウ","ェ","エ","ォ","オ",
    "カ","ガ","キ","ギ","ク","グ","ケ","ゲ","コ","ゴ",
    "サ","ザ","シ","ジ","ス","ズ","セ","ゼ","ソ","ゾ",
    "タ","ダ","チ","ヂ","ッ","ツ","ヅ","テ","デ","ト",
    "ド","ナ","ニ","ヌ","ネ","ノ","ハ","バ","パ","ヒ",
    "ビ","ピ","フ","ブ","プ","ヘ","ベ","ペ","ホ","ボ",
    "ポ","マ","ミ","ム","メ","モ","ャ","ヤ","ュ","ユ",
    "ョ","ヨ","ラ","リ","ル","レ","ロ","ヮ","ワ","ヲ",
    "ン","ヴ","ヵ","ヶ"
};

static const char hiraganas[][4] = {
    "ぁ","あ","ぃ","い","ぅ","う","ぇ","え","ぉ","お",
    "か","が","き","ぎ","く","ぐ","け","げ","こ","ご",
    "さ","ざ","し","じ","す","ず","せ","ぜ","そ","ぞ",
    "た","だ","ち","ぢ","っ","つ","づ","て","で","と",
    "ど","な","に","ぬ","ね","の","は","ば","ぱ","ひ",
    "び","ぴ","ふ","ぶ","ぷ","へ","べ","ぺ","ほ","ぼ",
    "ぽ","ま","み","む","め","も","ゃ","や","ゅ","ゆ",
    "ょ","よ","ら","り","る","れ","ろ","ゎ","わ","を",
    "ん","ゔ","ゕ","ゖ"
};

Furigana::Furigana() {
    /* Initialize the katakana to hiragana hash table. */
    static size_t numKanas = sizeof(katakanas)/sizeof(katakanas[0]);
    for (int i = 0; i < numKanas; i++) {
        uint32_t katakana = *(uint32_t*)katakanas[i];
        uint32_t hiragana = *(uint32_t*)hiraganas[i];
        Furigana::KataToHiraTable.insert(std::make_pair(katakana, hiragana));
    }
}

/**
 * Return the given string with katakanas converted
 * to their hiragana equivalents.
 */
std::string Furigana::katakana_to_hiragana(std::string katakana)
{
    const char *str = katakana.c_str();
    char character[5] = { '\0' }; /* 4 UTF-8 bytes + '\0' = 5 */
    std::string hiraganas;

    while (utf8_getc(&str, character, sizeof(character)-1)) {
        auto res = Furigana::KataToHiraTable.find(*(uint32_t*)character);
        if (res != Furigana::KataToHiraTable.end()) {
            std::memcpy(character, &res->second, sizeof(character)-1);
        }

        hiraganas.append(character, std::strlen(character));
    }

    return hiraganas;
}

static inline void remove_spaces(std::string &str)
{
    str.erase(
        std::remove_if(
            str.begin(),
            str.end(),
            (int(*)(int))std::isspace),
        str.end()
    );
}

/**
 * Given two strings, check if they start with the same characters. If so,
 * return the length (in bytes) of equal part, or zero otherwise.
 * Return zero if the strings are equal.
 */
static size_t find_initial_equal_chars(char *kanjis_start, char *reading_start)
{
    const char *kanjis  = (const char*) kanjis_start;
    const char *reading = (const char*) reading_start;
    const char *kanjis_prev;
    char kanji_char[5] = { '\0' }; /* 4 UTF-8 bytes + '\0' = 5 */
    char  kana_char[5] = { '\0' }; /* 4 UTF-8 bytes + '\0' = 5 */

    if (strcmp(kanjis, reading) == 0) {
        return 0;
    }

    while (true) {
        kanjis_prev = kanjis;
        int more_kanjis  = utf8_getc(&kanjis, kanji_char, sizeof(kanji_char)-1);
        int more_reading = utf8_getc(&reading, kana_char, sizeof(kana_char)-1);
        if (!more_kanjis || !more_reading ||
            strncmp(kanji_char, kana_char, sizeof(kanji_char)) != 0) {
            break;
        }
    }
    return kanjis_prev - kanjis_start;
}

/**
 * Split the given string into one or more parts,
 * giving the provided separator character.
 * Each token in readingString matches one char of kanjisString.
 * Several consecutive separator characters means the token
 * spans on multiple chars of kanjisString.
 */
static std::vector<std::pair<std::string, std::string> > split_furigana(
    std::string kanjisString,
    std::string readingString,
    const char separator
) {
    std::vector<std::pair<std::string, std::string> > tokens;
    const char *kanjis = kanjisString.c_str();
    std::size_t reading_start = 0, reading_end = 0;
    char kanji_char[5] = { '\0' }; /* 4 UTF-8 bytes + '\0' = 5 */

    while (true) {
        int more_kanjis = utf8_getc(&kanjis, kanji_char, sizeof(kanji_char)-1);
        std::string kanji_chars(kanji_char);
        reading_end = readingString.find(separator, reading_start);
        std::string reading_chars
            = readingString.substr(reading_start, reading_end - reading_start);

        /* Consecutive separators: the reading spans on the previous kanji */
        if (reading_start == reading_end) {
            kanji_chars = tokens.back().first + kanji_chars;
            reading_chars = tokens.back().second;
            tokens.pop_back();
        }

        /* Stuff the remaining kanjis if we reached the end of the readings */
        if (reading_end == std::string::npos && more_kanjis) {
            while (more_kanjis = utf8_getc(&kanjis, kanji_char,
                                           sizeof(kanji_char)-1)) {
                kanji_chars += kanji_char;
            }
        }

        tokens.push_back(std::pair<std::string, std::string>(
            kanji_chars,
            reading_chars
        ));
        if (!more_kanjis || reading_end == std::string::npos) {
            break;
        }
        reading_start = reading_end + 1;
    }
    return tokens;
}

/**
 * Removes useless furiganas at the beginning and at the end.
 */
std::vector<std::pair<std::string, std::string> > Furigana::tokenize(
    std::string kanjisString,
    std::string readingString
) {
    size_t start_len, end_len;

    remove_spaces(readingString);
    readingString = this->katakana_to_hiragana(readingString);

    auto tokens = split_furigana(
        kanjisString,
        readingString,
        WARIFURI_SEPARATOR
    );

    for (auto& text : tokens) {
        if (this->katakana_to_hiragana(text.first) == text.second) {
            text.second = "";
        }
    }

    return tokens;
}
