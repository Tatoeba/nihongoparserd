#ifndef SINOPARSER_SERVER_H
#define SINOPARSER_SERVER_H

#include <iostream>

namespace MeCab {
    class Tagger;
}


class Server {

    public:
        // TODO : I know it's bad, shame on me
        MeCab::Tagger* wakatiTagger;
        MeCab::Tagger* yomiTagger;

        Server(std::string address, int port);
        ~Server();
};


#endif

