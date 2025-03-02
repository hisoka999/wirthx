//
// Created by stefan on 02.03.25.
//

#ifndef LANGUAGESERVER_H
#define LANGUAGESERVER_H

#include <map>
#include <string>

struct LspDocument
{
    std::string uri;
    std::string text;
};

class LanguageServer
{
    std::map<std::string, LspDocument> m_openDocuments;

public:
    explicit LanguageServer();

    void handleRequest();
};


#endif // LANGUAGESERVER_H
