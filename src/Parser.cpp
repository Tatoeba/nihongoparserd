
#include <string.h>

#include "Parser.h"

/* Number of the reading field in the IPA dictionary. */
#define IPA_FEATURE_READING_FIELD 8

/**
 * Return the wanted_field'th field of the feature of the given node.
 */
static std::string mecab_node_get_field(
    const MeCab::Node *node,
    int wanted_field
) {
    size_t field = 1;
    char *token = strdupa(node->feature);

    while (token != NULL) {
        char *next = NULL;

        if (*token == '"') {
            char *closing_quote = strchr(token + 1, '"');
            if (closing_quote && closing_quote[1] == ',') {
                next = closing_quote + 1;
                token++; // skip opening quote
                *closing_quote = '\0'; // remove closing quote
            }
        }
        if (!next)
            next = strchr(token, ',');
        if (next) {
            *next = '\0';
            next++;
        }

        if (field == wanted_field) {
            return std::string(token);
        }
        field++;
        token = next;
    }
    return "";
}

/**
 * Return the reading field of the given node (assuming IPA dictionary).
 */
static std::string mecab_node_get_reading(const MeCab::Node *node) {
    return mecab_node_get_field(node, IPA_FEATURE_READING_FIELD);
}

/**
 * Tokenize the given string into a list of (writing, reading) pairs,
 * for instance "私はここにいる。" should returns:
 * (
 *   ("私",   "わたし"),
 *   ("は",   "は"),
 *   ("ここ", "ここ"),
 *   ("に",   "に"),
 *   ("いる", "いる"),
 *   ("。",   "。")
 * )
 */
std::vector<std::pair<std::string, std::string> > Parser::tokenize(
    char const *str
) {
    std::vector<std::pair<std::string, std::string> > tokens;
    const MeCab::Node* node = this->tagger->parseToNode(str);
    for (; node; node = node->next) {
        if (node->stat != MECAB_BOS_NODE && node->stat != MECAB_EOS_NODE) {
            std::string token = std::string(node->surface, node->length);
            std::string reading = std::string(mecab_node_get_reading(node));

            if (Parser::ignoredReadings.count(token)) {
                reading = "";
            }

            tokens.push_back(std::pair<std::string, std::string>(
                token,
                reading
            ));
        }
    }
    return tokens;
}

Parser::Parser() {
    Parser::ignoredReadings = {
        "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
        "\uff10", "\uff11", "\uff12", "\uff13", "\uff14",
        "\uff15", "\uff16", "\uff17", "\uff18", "\uff19"
    };
    wakatiTagger = MeCab::createTagger("-Owakati");
    yomiTagger = MeCab::createTagger("-Oyomi");
    tagger = MeCab::createTagger("");
}

Parser::~Parser() {
    delete wakatiTagger;
    delete yomiTagger;
    delete tagger;
}

