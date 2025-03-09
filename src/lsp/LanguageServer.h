//
// Created by stefan on 02.03.25.
//

#ifndef LANGUAGESERVER_H
#define LANGUAGESERVER_H

#include <map>
#include <string>
#include "compiler/CompilerOptions.h"
struct LspDocument
{
    std::string uri;
    std::string text;
};

class LanguageServer
{
    std::map<std::string, LspDocument> m_openDocuments;
    CompilerOptions m_options;

public:
    explicit LanguageServer(CompilerOptions options);

    void handleRequest();
};


#endif // LANGUAGESERVER_H
