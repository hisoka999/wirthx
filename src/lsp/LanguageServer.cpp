//
// Created by stefan on 02.03.25.
//

#include "LanguageServer.h"
#include <llvm-18/llvm/Support/Endian.h>
#include <llvm/Support/CommandLine.h>

#include <iostream>
#include <llvm/Support/JSON.h>
#include "Lexer.h"
#include "Parser.h"

struct NotificationMessage
{
    std::string method;
    std::vector<std::string> args;
};

LanguageServer::LanguageServer() {}
void sendNotification(std::string method)
{
    std::string resultString;

    llvm::raw_string_ostream sstream(resultString);
    llvm::json::Object response;
    response["jsonrpc"] = "2.0";
    response["method"] = method;
    llvm::json::Value resultValue(std::move(response));
    sstream << resultValue;
    std::cerr << "TEST: " << sstream.str() << "\n";
    std::cout << "Content-Length: " << sstream.str().length() << "\r\n\r\n";
    std::cout << sstream.str() << "\n";
}

llvm::json::Object buildPosition(int line, int character)
{
    llvm::json::Object response;
    response["line"] = line - 1;
    response["character"] = character - 1;
    return response;
}

llvm::json::Object buildColor(TokenType token)
{
    llvm::json::Object response;
    response["red"] = 1.0;
    response["green"] = 0.0;
    response["blue"] = 1.0;
    response["alpha"] = 1.0;
    return response;
}
void LanguageServer::handleRequest()
{
    while (true)
    {


        std::string commandString;
        getline(std::cin, commandString);
        auto length = std::atoi(commandString.substr(16).c_str());
        std::cerr << " length: " << length << std::endl;
        getline(std::cin, commandString); // empty line
        std::vector<char> buffer;
        buffer.resize(length);
        std::cin.read(&buffer[0], length);
        commandString = std::string(buffer.begin(), buffer.end());

        auto request = llvm::json::parse(commandString);
        if (!request)
        {
            std::cerr << "Failed to parse request" << std::endl;
            std::cerr << " command: " << commandString << std::endl;
            continue;
        }
        auto requestObject = request.get().getAsObject();
        auto method = requestObject->getString("method");
        if (method)
        {
            std::string resultString;

            std::cerr << method.value().str() << std::endl;
            llvm::raw_string_ostream sstream(resultString);
            llvm::json::Object response;
            response["jsonrpc"] = "2.0";
            response["id"] = requestObject->getString("id");
            llvm::json::Object result;
            if (method.value() == "shutdown")
            {
                // sendNotification("exit");
                break;
            }
            if (method.value() == "initialize")
            {
                llvm::json::Object capabilities;
                capabilities["documentHighlightProvider"] = true;
                capabilities["documentSymbolProvider"] = true;
                capabilities["colorProvider"] = true;
                llvm::json::Object diagnosticProvider;
                diagnosticProvider["interFileDependencies"] = true;
                capabilities["diagnosticProvider"] = std::move(diagnosticProvider);
                llvm::json::Object textDocumentSync;
                textDocumentSync["openClose"] = true;
                textDocumentSync["change"] = 1;
                capabilities["textDocumentSync"] = std::move(textDocumentSync);
                result["capabilities"] = std::move(capabilities);
                llvm::json::Object serverInfo;
                serverInfo["name"] = "wirthx";
                serverInfo["version"] = "0.1";
                result["serverInfo"] = std::move(serverInfo);
                response["result"] = std::move(result);
            }
            else if (method.value() == "initialized")
            {
                response["result"] = std::move(result);
                continue;
            }
            else if (method.value() == "textDocument/didOpen")
            {
                auto params = requestObject->getObject("params");
                auto uri = params->getObject("textDocument")->getString("uri");

                auto text = params->getObject("textDocument")->getString("text");
                m_openDocuments[uri.value().str()] = LspDocument{.uri = uri.value().str(), .text = text.value().str()};
                response["result"] = std::move(result);
            }
            else if (method.value() == "textDocument/didChange")
            {
                auto params = requestObject->getObject("params");
                auto uri = params->getObject("textDocument")->getString("uri");

                auto text = params->getArray("contentChanges")->front().getAsObject()->getString("text");
                m_openDocuments[uri.value().str()] = LspDocument{.uri = uri.value().str(), .text = text.value().str()};
                response["result"] = std::move(result);
            }
            else if (method.value() == "textDocument/documentColor")
            {
                auto params = requestObject->getObject("params");
                auto uri = params->getObject("textDocument")->getString("uri");
                auto &document = m_openDocuments.at(uri.value().str());
                Lexer lexer;
                auto tokens = lexer.tokenize(uri.value().str(), document.text);
                llvm::json::Array array;
                // for (auto &token: tokens)
                // {
                //     if (token.tokenType == TokenType::KEYWORD)
                //     {
                //         llvm::json::Object colorResult;
                //         llvm::json::Object range;
                //         range["start"] = buildPosition(token.row, token.col);
                //         range["end"] = buildPosition(token.row, token.col +
                //         token.sourceLocation.byte_offset); colorResult["range"] = std::move(range);
                //         colorResult["color"] = buildColor(token.tokenType);
                //         array.push_back(std::move(colorResult));
                //     }
                // }

                response["result"] = std::move(array);
            }
            else
            {
                std::cerr << "unsupported method: " << method.value().str() << std::endl;
                continue;
            }

            if (requestObject->getString("id").has_value())
            {
                llvm::json::Value resultValue(std::move(response));
                sstream << resultValue;
                std::cerr << "TEST: " << sstream.str() << "\n";
                std::cout << "Content-Length: " << sstream.str().length() << "\r\n\r\n";
                std::cout << sstream.str();
            }
        }
    }
}
