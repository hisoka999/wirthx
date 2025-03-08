//
// Created by stefan on 02.03.25.
//

#include "LanguageServer.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Endian.h>

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
void sendNotification(const std::string &method)
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

llvm::json::Object buildPosition(const int line, const int character)
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
void sendLogMessage(const std::vector<std::string> &messages)
{
    if (messages.empty())
        return;
    llvm::json::Array logMessages;
    for (auto &message: messages)
    {
        llvm::json::Object logMessage;
        logMessage["message"] = message;
        logMessage["type"] = 1;
        logMessages.push_back(std::move(logMessage));
    }
    llvm::json::Object response;
    response["jsonrpc"] = "2.0";
    response["method"] = "window/logMessage";
    response["params"] = std::move(logMessages);
    llvm::json::Value resultValue(std::move(response));
    std::string resultString;

    llvm::raw_string_ostream sstream(resultString);
    sstream << resultValue;
    std::cerr << "TEST: " << sstream.str() << "\n";
    std::cout << "Content-Length: " << sstream.str().length() << "\r\n\r\n";
    std::cout << sstream.str();
}

constexpr int mapOutputTypeToSeverity(const OutputType output)
{
    switch (output)
    {
        case OutputType::ERROR:
            return 1;
        case OutputType::HINT:
            return 4;
        case OutputType::WARN:
            return 2;
    }
    return 0;
}
void sentDiagnostics(std::vector<ParserError> &errors)
{
    if (errors.empty())
        return;

    // group messages by file
    std::map<std::string, std::vector<ParserError>> errorsMap;
    for (auto &error: errors)
    {
        errorsMap[error.token.sourceLocation.filename].push_back(error);
    }

    for (auto &[fileName, messsages]: errorsMap)
    {
        llvm::json::Array logMessages;
        for (const auto &[outputType, token, message]: messsages)
        {
            llvm::json::Object logMessage;
            llvm::json::Object range;
            range["start"] = buildPosition(token.row, token.col);
            range["end"] = buildPosition(token.row, token.col + token.sourceLocation.num_bytes);
            logMessage["range"] = std::move(range);
            logMessage["severity"] = mapOutputTypeToSeverity(outputType);
            logMessage["message"] = message;
            llvm::json::Array relatedInformations;
            llvm::json::Object source;
            llvm::json::Object location;
            location["uri"] = token.sourceLocation.filename;
            llvm::json::Object range2;
            range2["start"] = buildPosition(token.row, token.col);
            range2["end"] = buildPosition(token.row, token.col + token.sourceLocation.num_bytes);
            location["range"] = std::move(range2);
            source["location"] = std::move(location);
            source["message"] = message;
            source["source"] = "wirthx";
            relatedInformations.push_back(std::move(source));

            logMessage["relatedInformation"] = std::move(relatedInformations);
            logMessages.push_back(std::move(logMessage));
        }
        llvm::json::Object response;
        response["jsonrpc"] = "2.0";
        response["method"] = "textDocument/publishDiagnostics";
        llvm::json::Array diagnosticsParams;
        {
            llvm::json::Object PublishDiagnosticsParams;
            PublishDiagnosticsParams["uri"] = fileName;
            PublishDiagnosticsParams["diagnostics"] = std::move(logMessages);
            diagnosticsParams.push_back(std::move(PublishDiagnosticsParams));
        }
        response["params"] = std::move(diagnosticsParams);
        llvm::json::Value resultValue(std::move(response));
        std::string resultString;

        llvm::raw_string_ostream sstream(resultString);
        sstream << resultValue;
        std::cerr << "TEST: " << sstream.str() << "\n";
        std::cout << "Content-Length: " << sstream.str().length() << "\r\n\r\n";
        std::cout << sstream.str();
    }
}

void LanguageServer::handleRequest()
{
    std::vector<std::string> logMessages;
    std::vector<ParserError> errors;
    while (true)
    {
        logMessages.clear();
        errors.clear();

        std::string commandString;
        getline(std::cin, commandString);
        auto length = std::atoi(commandString.substr(16).c_str());
        // std::cerr << " length: " << length << std::endl;
        getline(std::cin, commandString); // empty line
        std::vector<char> buffer;
        buffer.resize(length);
        std::cin.read(&buffer[0], length);
        commandString = std::string(buffer.begin(), buffer.end());

        auto request = llvm::json::parse(commandString);
        if (!request)
        {
            logMessages.push_back("Failed to parse request");
            logMessages.push_back(" command: " + commandString);
            continue;
        }
        else
        {
            std::cerr << "command: " << commandString << std::endl;
        }

        auto requestObject = request.get().getAsObject();
        auto method = requestObject->getString("method");
        if (method)
        {
            std::string resultString;

            logMessages.push_back("method: " + method.value().str());
            llvm::raw_string_ostream sstream(resultString);
            llvm::json::Object response;
            response["jsonrpc"] = "2.0";
            bool hasId = false;
            if (requestObject->getInteger("id").has_value())
            {
                response["id"] = requestObject->getInteger("id").value();
                hasId = true;
            }
            else if (requestObject->getString("id").has_value())
            {
                response["id"] = requestObject->getString("id").value();
                hasId = true;
            }
            llvm::json::Object result;
            if (method.value() == "shutdown")
            {
                // sendNotification("exit");
                break;
            }
            if (method.value() == "initialize")
            {
                llvm::json::Object capabilities;
                capabilities["documentHighlightProvider"] = false;
                capabilities["documentSymbolProvider"] = true;
                capabilities["colorProvider"] = true;
                llvm::json::Object diagnosticProvider;
                diagnosticProvider["interFileDependencies"] = true;
                diagnosticProvider["workspaceDiagnostics"] = true;
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
                Lexer lexer;
                auto tokens = lexer.tokenize(uri.value().str(), text.value().str());
                std::filesystem::path filePath = uri.value().str();
                std::vector<std::filesystem::path> rtlDirectories;
                rtlDirectories.push_back(std::filesystem::current_path() / "rtl");
                std::unordered_map<std::string, bool> definitions;
                Parser parser(rtlDirectories, filePath, definitions, tokens);
                auto ast = parser.parseFile();
                for (auto error: parser.getErrors())
                {
                    errors.emplace_back(error);
                }
            }
            else if (method.value() == "textDocument/didChange")
            {
                auto params = requestObject->getObject("params");
                auto uri = params->getObject("textDocument")->getString("uri");

                auto text = params->getArray("contentChanges")->front().getAsObject()->getString("text");
                m_openDocuments[uri.value().str()] = LspDocument{.uri = uri.value().str(), .text = text.value().str()};
                response["result"] = std::move(result);
                Lexer lexer;
                auto tokens = lexer.tokenize(uri.value().str(), text.value().str());
                std::filesystem::path filePath = uri.value().str();
                std::vector<std::filesystem::path> rtlDirectories;
                rtlDirectories.push_back(std::filesystem::current_path() / "rtl");
                std::unordered_map<std::string, bool> definitions;
                Parser parser(rtlDirectories, filePath, definitions, tokens);
                auto ast = parser.parseFile();
                for (auto error: parser.getErrors())
                {
                    errors.emplace_back(error);
                }
            }
            else if (method.value() == "textDocument/documentHighlight")
            {
                std::cerr << " command: " << commandString << "\n";

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
                std::cerr << "unsupported method: " << method.value().str() << "\n";
                logMessages.push_back("unsupported method: " + method.value().str());
                continue;
            }

            if (hasId)
            {
                llvm::json::Value resultValue(std::move(response));
                sstream << resultValue;
                std::cout << "Content-Length: " << sstream.str().length() << "\r\n\r\n";
                std::cout << sstream.str();
                std::cerr << sstream.str();
            }
        }
        else
        {
            logMessages.push_back("method not found");
        }

        sendLogMessage(logMessages);
        sentDiagnostics(errors);
    }
}
