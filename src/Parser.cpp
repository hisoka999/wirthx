#include "Parser.h"
#include <algorithm>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <magic_enum/magic_enum.hpp>
#include <utility>
#include "Lexer.h"
#include "ast/ArrayAccessNode.h"
#include "ast/ArrayAssignmentNode.h"
#include "ast/BinaryOperationNode.h"
#include "ast/BlockNode.h"
#include "ast/BooleanNode.h"
#include "ast/BreakNode.h"
#include "ast/CharConstantNode.h"
#include "ast/ComparissionNode.h"
#include "ast/FieldAccessNode.h"
#include "ast/FieldAssignmentNode.h"
#include "ast/ForNode.h"
#include "ast/FunctionCallNode.h"
#include "ast/FunctionDefinitionNode.h"
#include "ast/IfConditionNode.h"
#include "ast/LogicalExpressionNode.h"
#include "ast/NumberNode.h"
#include "ast/RecordType.h"
#include "ast/RepeatUntilNode.h"
#include "ast/ReturnNode.h"
#include "ast/StringConstantNode.h"
#include "ast/SystemFunctionCallNode.h"
#include "ast/VariableAccessNode.h"
#include "ast/VariableAssignmentNode.h"
#include "ast/WhileNode.h"
#include "compare.h"


Parser::Parser(const std::vector<std::filesystem::path> &rtlDirectories, std::filesystem::path path,
               const std::vector<Token> &tokens) :
    m_rtlDirectories(std::move(rtlDirectories)), m_file_path(std::move(path)), m_tokens(tokens)
{
    m_typeDefinitions["shortint"] = VariableType::getInteger(8);
    m_typeDefinitions["byte"] = VariableType::getInteger(8);
    m_typeDefinitions["char"] = VariableType::getInteger(8);
    m_typeDefinitions["smallint"] = VariableType::getInteger(16);
    m_typeDefinitions["word"] = VariableType::getInteger(16);
    m_typeDefinitions["longint"] = VariableType::getInteger();
    m_typeDefinitions["integer"] = VariableType::getInteger();
    m_typeDefinitions["int64"] = VariableType::getInteger(64);
    m_typeDefinitions["string"] = VariableType::getString();
    m_typeDefinitions["boolean"] = VariableType::getBoolean();
}

bool Parser::hasNext() const { return m_current < m_tokens.size() - 1; }

Token Parser::current() { return m_tokens[m_current]; }

Token Parser::next()
{
    if (hasNext())
    {
        m_current++;
    }
    return current();
}

bool Parser::canConsume(const TokenType tokenType, const size_t next) const
{
    return m_tokens[m_current + next].tokenType == tokenType;
}
bool Parser::canConsume(const TokenType tokenType) const { return canConsume(tokenType, 1); }

bool Parser::tryConsume(const TokenType tokenType)
{
    if (canConsume(tokenType))
    {
        next();
        return true;
    }
    return false;
}
bool Parser::consume(const TokenType tokenType)
{
    if (canConsume(tokenType))
    {
        next();
        return true;
    }
    else
    {
        m_errors.push_back(ParserError{
                .file_name = m_file_path.string(),
                .token = m_tokens[m_current + 1],
                .message = "expected token  '" + std::string(magic_enum::enum_name(tokenType)) + "' but found " +
                           std::string(magic_enum::enum_name(m_tokens[m_current + 1].tokenType)) + "!"});
        throw ParserException(m_errors);
    }
    return false;
}

bool Parser::consumeKeyWord(const std::string &keyword)
{
    if (tryConsumeKeyWord(keyword))
    {
        return true;
    }
    m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                   .token = m_tokens[m_current + 1],
                                   .message = "expected keyword  '" + keyword + "' but found " +
                                              std::string(m_tokens[m_current + 1].lexical) + "!"});
    throw ParserException(m_errors);
}

bool Parser::canConsumeKeyWord(const std::string &keyword) const
{
    return canConsume(TokenType::KEYWORD) && iequals(m_tokens[m_current + 1].lexical, keyword);
}

bool Parser::tryConsumeKeyWord(const std::string &keyword)
{
    if (canConsumeKeyWord(keyword))
    {
        next();
        return true;
    }
    return false;
}

std::optional<std::shared_ptr<VariableType>> Parser::determinVariableTypeByName(const std::string &name) const
{
    for (const auto &[defName, definition]: m_typeDefinitions)
    {
        if (iequals(defName, name))
            return definition;
    }

    return std::nullopt;
}

std::shared_ptr<ASTNode> Parser::parseEscapedString(const Token &token)
{
    auto result = std::string{};
    size_t x = 1;
    while (x < token.lexical.size())
    {
        auto next = token.lexical.find('#', x);
        if (next == std::string::npos)
            next = token.lexical.size();
        auto tmp = token.lexical.substr(x, next - x);
        result += std::atoi(tmp.data());
        x = next + 1;
    }
    return std::make_shared<StringConstantNode>(result);
}
std::shared_ptr<ASTNode> Parser::parseFunctionCall(const Token &token, size_t currentScope)
{
    auto functionName = std::string(token.lexical);
    const bool isSysCall = isKnownSystemCall(functionName);
    if (!isSysCall && std::ranges::find(m_known_function_names, functionName) == std::end(m_known_function_names))
    {
        for (auto &fun: m_known_function_names)
            std::cerr << "func: " << fun << "\n";
        m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                       .token = token,
                                       .message = "a function with the name '" + std::string(token.lexical) +
                                                  "' is not yet defined!"});
    }
    consume(TokenType::LEFT_CURLY);

    // Token subToken = next();
    std::vector<std::shared_ptr<ASTNode>> callArgs;

    if (!tryConsume(TokenType::RIGHT_CURLY))
    {
        do
        {
            if (tryConsume(TokenType::COMMA) || current().tokenType == TokenType::COMMA)
            {
                next();
            }

            if (auto node = parseExpression(current(), currentScope))
                callArgs.push_back(node);
        }
        while (!tryConsume(TokenType::RIGHT_CURLY));
    }

    // while (subToken.tokenType != TokenType::RIGHT_CURLY && subToken.tokenType != TokenType::SEMICOLON)
    // {
    //     switch (subToken.tokenType)
    //     {
    //         case TokenType::CHAR:
    //         case TokenType::NUMBER:
    //         case TokenType::ESCAPED_STRING:
    //         case TokenType::STRING:
    //         case TokenType::NAMEDTOKEN:
    //         case TokenType::MINUS:
    //         {
    //             auto node = parseExpression(subToken, currentScope);
    //             subToken = current();
    //             callArgs.push_back(node);
    //         }
    //         break;
    //         case TokenType::COMMA:
    //             break;
    //         default:
    //             m_errors.push_back(ParserError{.file_name = m_file_path.string(),
    //                                            .token = subToken,
    //                                            .message = "unexpected token in function call"});
    //
    //             throw ParserException(m_errors);
    //     }
    //     if (subToken.tokenType == TokenType::RIGHT_CURLY)
    //     {
    //         tryConsume(TokenType::RIGHT_CURLY);
    //         break;
    //     }
    //     subToken = next();
    // }


    if (isSysCall)
    {
        return std::make_shared<SystemFunctionCallNode>(functionName, callArgs);
    }
    return std::make_shared<FunctionCallNode>(functionName, callArgs);
}
std::shared_ptr<ASTNode> Parser::parseToken(const Token &token, const size_t currentScope,
                                            std::vector<std::shared_ptr<ASTNode>> nodes)
{
    std::vector<std::shared_ptr<ASTNode>> args;
    switch (token.tokenType)
    {
        case TokenType::NUMBER:
        {
            return parseNumber(token, currentScope);
        }
        case TokenType::ESCAPED_STRING:
        {
            return parseEscapedString(token);
        }
        case TokenType::STRING:
        {
            return std::make_shared<StringConstantNode>(std::string(token.lexical));
        }
        case TokenType::CHAR:
        {
            return std::make_shared<CharConstantNode>(token.lexical);
        }
        case TokenType::PLUS:
        {
            auto rhs = parseToken(next(), currentScope, {});
            auto lhs = nodes[0];
            return std::make_shared<BinaryOperationNode>(Operator::PLUS, lhs, rhs);
        }
        case TokenType::MINUS:
        {
            auto rhs = parseToken(next(), currentScope, {});
            auto lhs = nodes[0];
            return std::make_shared<BinaryOperationNode>(Operator::MINUS, lhs, rhs);
        }
        case TokenType::MUL:
        {
            auto rhs = parseToken(next(), currentScope, {});
            auto lhs = nodes[0];
            return std::make_shared<BinaryOperationNode>(Operator::MUL, lhs, rhs);
        }
        case TokenType::DIV:
        {
            auto rhs = parseToken(next(), currentScope, {});
            auto lhs = nodes[0];
            return std::make_shared<BinaryOperationNode>(Operator::DIV, lhs, rhs);
        }
        case TokenType::NAMEDTOKEN:
        {


            if (canConsume(TokenType::LEFT_CURLY))
            {
                return parseFunctionCall(token, currentScope);
            }
            if (canConsume(TokenType::LEFT_SQUAR))
            {
                const Token arrayName = token;
                if (!isVariableDefined(token.lexical, currentScope))
                {
                    m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                                   .token = token,
                                                   .message = "A variable with the name '" +
                                                              std::string(token.lexical) + "' is not yet defined!"});
                    return nullptr;
                }
                consume(TokenType::LEFT_SQUAR);
                auto indexNode = parseToken(next(), currentScope, {});
                consume(TokenType::RIGHT_SQUAR);
                return std::make_shared<ArrayAccessNode>(TokenWithFile{.token = arrayName, .fileName = m_file_path},
                                                         indexNode);
            }
            if (canConsume(TokenType::DOT))
            {
                Token elementName = token;
                if (!isVariableDefined(token.lexical, currentScope))
                {
                    m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                                   .token = token,
                                                   .message = "A variable with the name '" +
                                                              std::string(token.lexical) + "' is not yet defined!"});
                    return nullptr;
                }
                consume(TokenType::DOT);
                consume(TokenType::NAMEDTOKEN);
                Token field = current();
                return std::make_shared<FieldAccessNode>(TokenWithFile{.token = elementName, .fileName = m_file_path},
                                                         TokenWithFile{.token = field, .fileName = m_file_path}

                );
            }
            else
            {
                if (!isVariableDefined(token.lexical, currentScope))
                {
                    m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                                   .token = token,
                                                   .message = "A variable with the name '" +
                                                              std::string(token.lexical) + "' is not yet defined!"});
                    return nullptr;
                }
                return std::make_shared<VariableAccessNode>(token.lexical);
            }

            break;
        }
        case TokenType::KEYWORD:
        {
            if (iequals(token.lexical, "true"))
            {
                return std::make_shared<BooleanNode>(true);
            }
            if (iequals(token.lexical, "false"))
            {
                return std::make_shared<BooleanNode>(false);
            }
            if (iequals(token.lexical, "mod"))
            {
                auto rhs = parseToken(next(), currentScope, {});

                if (nodes.empty())
                {
                    m_errors.push_back(
                            ParserError{.file_name = m_file_path.string(),
                                        .token = token,
                                        .message = "the left hand side of the modulus operation is missing"});
                    return nullptr;
                }

                auto lhs = nodes[0];
                return std::make_shared<BinaryOperationNode>(Operator::MOD, lhs, rhs);
            }
        }
        default:
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = token,
                                           .message = std::string("Token type '" +
                                                                  std::string(magic_enum::enum_name(token.tokenType)) +
                                                                  "' not yet implemented")});
            throw ParserException(m_errors);
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseNumber(const Token &token, size_t currentScope)
{
    auto value = std::atoll(token.lexical.data());
    auto base = 1 + static_cast<int>(std::log2(value));
    base = (base > 32) ? 64 : 32;
    auto lhs = std::make_shared<NumberNode>(value, base);
    if (canConsume(TokenType::PLUS) || canConsume(TokenType::MINUS) || canConsumeKeyWord("mod"))
    {
        return parseToken(next(), currentScope, {lhs});
    }
    return lhs;
}

bool Parser::isVariableDefined(const std::string_view name, size_t scope)
{
    for (const auto &def: m_known_variable_definitions)
    {
        if (iequals(def.variableName, name) && def.scopeId <= scope)
            return true;
    }
    return false;
}

std::vector<VariableDefinition> Parser::parseVariableDefinitions(const Token &token, size_t scope)
{
    std::vector<VariableDefinition> result;
    Token _currentToken = token;

    if (canConsume(TokenType::ENDLINE))
    {
        consume(TokenType::ENDLINE);
        return {};
    }
    // consume var declarations


    std::vector<std::string> varNames;
    do
    {
        consume(TokenType::NAMEDTOKEN);
        _currentToken = current();
        auto varName = std::string(_currentToken.lexical);
        varNames.push_back(varName);
        if (!tryConsume(TokenType::COMMA))
        {
            break;
        }
    }
    while (!canConsume(TokenType::COLON));

    std::optional<std::shared_ptr<VariableType>> type;
    std::string varType;

    if (tryConsume(TokenType::COLON))
    {
        if (tryConsume(TokenType::NAMEDTOKEN))
        {
            _currentToken = current();
            varType = std::string(_currentToken.lexical);
            type = determinVariableTypeByName(varType);
        }
        else if (tryConsumeKeyWord("array"))
        {
            varType = "array";
            type = parseArray(scope, false);
        }
    }
    std::shared_ptr<ASTNode> value;
    if (tryConsume(TokenType::EQUAL))
    {
        value = parseToken(next(), scope, {});

        // determin the type from the parsed token
        if (!type)
        {
            type = value->resolveType(nullptr, nullptr);
            if (type.has_value())
                varType = type.value()->typeName;
        }
    }

    consume(TokenType::SEMICOLON);
    consume(TokenType::ENDLINE);
    for (const auto &varName: varNames)
    {
        if (isVariableDefined(varName, scope))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = _currentToken,
                                           .message = "A variable or constant with the name " + varName +
                                                      " was allready defined!"});
            return {};
        }
        else if (!type.has_value())
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = _currentToken,
                                           .message = "A type " + varType + " of the variable " + varName +
                                                      " could not be determined!"});
            return {};
        }

        result.push_back(VariableDefinition{.variableType = type.value(),
                                            .variableName = varName,
                                            .scopeId = scope,
                                            .value = value,
                                            .constant = false});
    }
    return result;
}

std::shared_ptr<BlockNode> Parser::parseBlock(const Token &currentToken, size_t scope)
{
    Token _currentToken = currentToken;
    std::vector<VariableDefinition> blockVariableDefinitions;

    if (canConsumeKeyWord("const"))
    {
        consumeKeyWord("const");
        tryConsume(TokenType::ENDLINE);

        _currentToken = current();
        while (!canConsumeKeyWord("begin"))
        {
            if (canConsume(TokenType::ENDLINE))
            {
                consume(TokenType::ENDLINE);
                continue;
            }
            // consume var declarations
            consume(TokenType::NAMEDTOKEN);
            _currentToken = current();
            auto varName = std::string(_currentToken.lexical);


            std::optional<std::shared_ptr<VariableType>> type;
            std::string varType;

            if (tryConsume(TokenType::COLON))
            {
                consume(TokenType::NAMEDTOKEN);
                _currentToken = current();
                varType = std::string(_currentToken.lexical);
                type = determinVariableTypeByName(varType);
            }
            std::shared_ptr<ASTNode> value;
            if (consume(TokenType::EQUAL))
            {
                value = parseToken(next(), scope, {});

                // determin the type from the parsed token
                if (!type)
                {
                    type = value->resolveType(nullptr, nullptr);
                    if (type.has_value())
                        varType = type.value()->typeName;
                }
            }

            consume(TokenType::SEMICOLON);
            consume(TokenType::ENDLINE);
            if (isVariableDefined(varName, scope))
            {
                m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                               .token = currentToken,
                                               .message = "A variable or constant with the name " + varName +
                                                          " was allready defined!"});
                continue;
            }

            if (!type.has_value())
            {
                m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                               .token = currentToken,
                                               .message = "A type " + varType + " of the variable " + varName +
                                                          " could not be determined!"});
                continue;
            }

            m_known_variable_definitions.push_back(VariableDefinition{.variableType = type.value(),
                                                                      .variableName = varName,
                                                                      .scopeId = scope,
                                                                      .value = value,
                                                                      .constant = true});
            blockVariableDefinitions.push_back(VariableDefinition{.variableType = type.value(),
                                                                  .variableName = varName,
                                                                  .scopeId = scope,
                                                                  .value = value,
                                                                  .constant = true});
        }
    }

    if (canConsumeKeyWord("var"))
    {
        consumeKeyWord("var");
        tryConsume(TokenType::ENDLINE);

        _currentToken = current();
        while (!canConsumeKeyWord("begin"))
        {

            auto definitions = parseVariableDefinitions(_currentToken, scope);

            for (auto &definition: definitions)
            {
                m_known_variable_definitions.push_back(definition);
                blockVariableDefinitions.push_back(definition);
            }
        }
    }

    consumeKeyWord("begin");
    while (canConsume(TokenType::ENDLINE))
    {
        consume(TokenType::ENDLINE);
    }
    std::vector<std::shared_ptr<ASTNode>> nodes;
    while (!canConsumeKeyWord("end"))
    {
        if (current().tokenType == TokenType::ENDLINE)
        {
            next();
            continue;
        }
        auto expr = parseExpression(current(), scope);
        if (expr)
            nodes.emplace_back(expr);
        tryConsume(TokenType::SEMICOLON);
        tryConsume(TokenType::ENDLINE);
        if (current().tokenType == TokenType::T_EOF)
            break;
    }
    consumeKeyWord("end");
    return std::make_shared<BlockNode>(blockVariableDefinitions, nodes);
}

bool Parser::importUnit(std::vector<std::shared_ptr<ASTNode>> &nodes, std::string filename)
{
    auto path = this->m_file_path.parent_path() / filename;
    auto it = m_rtlDirectories.begin();
    while (!std::filesystem::exists(path))
    {
        if (m_rtlDirectories.end() == it)
            break;
        path = *it / filename;
        ++it;
    }
    Lexer lexer;
    std::ifstream file;
    std::istringstream is;
    file.open(path, std::ios::in);
    if (!file.is_open())
    {
        m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                       .token = current(),
                                       .message = std::string(current().lexical) + " is not a valid unit"});
        m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                       .token = current(),
                                       .message = path.string() + " is not a valid pascal file"});


        return true;
    }
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);

    auto tokens = lexer.tokenize(std::string_view{buffer});
    Parser parser(m_rtlDirectories, path, tokens);
    auto unit = parser.parseUnit();

    for (auto &error: parser.m_errors)
    {
        m_errors.push_back(error);
    }
    if (m_errors.size() > 0)
    {
        return true;
    }

    for (auto &definition: unit->getFunctionDefinitions())
    {
        bool functionExists = false;
        for (auto &node: nodes)
        {
            if (auto function = std::dynamic_pointer_cast<FunctionDefinitionNode>(node))
            {
                if (function->functionSignature() == definition->functionSignature())
                {
                    functionExists = true;
                    break;
                }
            }
        }
        if (!functionExists)
        {
            nodes.push_back(definition);
            m_known_function_names.push_back(definition->name());
        }
    }

    for (auto &[typeName, newType]: parser.getTypeDefinitions())
    {
        if (!m_typeDefinitions.contains(typeName))
        {
            m_typeDefinitions[typeName] = newType;
        }
    }

    for (auto &error: parser.m_errors)
    {
        m_errors.push_back(error);
    }
    return false;
}
std::map<std::string, std::shared_ptr<VariableType>> Parser::getTypeDefinitions() { return m_typeDefinitions; }
bool Parser::parseKeyWord(const Token &currentToken, std::vector<std::shared_ptr<ASTNode>> &nodes, size_t scope)
{
    bool parseOk = true;
    if (currentToken.lexical == "if")
    {
        auto conditionNode = parseExpression(next(), scope);
        consumeKeyWord("then");
        tryConsume(TokenType::ENDLINE);
        // consume(TokenType::RIGHT_CURLY);
        // parse expression list
        std::vector<std::shared_ptr<ASTNode>> ifExpressions;
        std::vector<std::shared_ptr<ASTNode>> elseExpressions;

        if (canConsumeKeyWord("begin") || canConsumeKeyWord("var"))
        {
            ifExpressions.push_back(parseBlock(current(), scope + 1));
        }
        else
        {

            ifExpressions.push_back(parseExpression(next(), scope));
            tryConsume(TokenType::SEMICOLON);
        }

        while (canConsume(TokenType::ENDLINE))
            tryConsume(TokenType::ENDLINE);
        if (tryConsumeKeyWord("else"))
        {
            tryConsume(TokenType::ENDLINE);
            if (canConsumeKeyWord("begin") || canConsumeKeyWord("var"))
            {
                elseExpressions.push_back(parseBlock(current(), scope + 1));
            }
            else
            {
                elseExpressions.push_back(parseExpression(next(), scope));
                tryConsume(TokenType::SEMICOLON);
            }
        }

        nodes.push_back(std::make_shared<IfConditionNode>(conditionNode, ifExpressions, elseExpressions));
    }
    else if (currentToken.lexical == "return")
    {
        auto returnExpression = parseExpression(next(), scope);
        nodes.push_back(std::make_shared<ReturnNode>(returnExpression));
    }
    else if (iequals(currentToken.lexical, "procedure"))
    {
        parseFunction(scope, nodes, true);
    }
    else if (iequals(currentToken.lexical, "function"))
    {
        parseFunction(scope, nodes, false);
    }
    else if (iequals(currentToken.lexical, "uses"))
    {
        while (consume(TokenType::NAMEDTOKEN))
        {
            auto filename = std::string(current().lexical) + ".pas";

            if (importUnit(nodes, filename))
                return false;

            if (!tryConsume(TokenType::COMMA))
                break;
        }
        consume(TokenType::SEMICOLON);
    }
    else if (iequals(currentToken.lexical, "while"))
    {
        auto expression = parseExpression(next(), scope + 1);
        std::vector<std::shared_ptr<ASTNode>> whileNodes;

        consumeKeyWord("do");

        while (canConsume(TokenType::ENDLINE))
        {
            consume(TokenType::ENDLINE);
        }
        while (!canConsumeKeyWord("begin") && !canConsumeKeyWord("var"))
        {
            parseKeyWord(next(), whileNodes, scope + 1);

            while (canConsume(TokenType::ENDLINE))
            {
                consume(TokenType::ENDLINE);
            }
        }
        whileNodes.push_back(parseBlock(current(), scope + 1));

        nodes.push_back(std::make_shared<WhileNode>(expression, whileNodes));
    }
    else if (iequals(currentToken.lexical, "for"))
    {
        consume(TokenType::NAMEDTOKEN);
        auto loopVariable = std::string(current().lexical);
        m_known_variable_definitions.push_back(VariableDefinition{
                .variableType = VariableType::getInteger(64), .variableName = loopVariable, .scopeId = scope + 1});
        consume(TokenType::COLON);
        consume(TokenType::EQUAL);
        auto loopStart = parseToken(next(), scope + 1, nodes);

        consumeKeyWord("to");
        auto loopEnd = parseToken(next(), scope + 1, nodes);

        std::vector<std::shared_ptr<ASTNode>> whileNodes;

        consumeKeyWord("do");

        while (canConsume(TokenType::ENDLINE))
        {
            consume(TokenType::ENDLINE);
        }
        while (!canConsumeKeyWord("begin") && !canConsumeKeyWord("var"))
        {
            parseKeyWord(next(), whileNodes, scope + 1);

            while (canConsume(TokenType::ENDLINE))
            {
                consume(TokenType::ENDLINE);
            }
        }
        whileNodes.push_back(parseBlock(current(), scope + 1));


        nodes.push_back(std::make_shared<ForNode>(loopVariable, loopStart, loopEnd, whileNodes));
    }
    else if (iequals(currentToken.lexical, "break"))
    {
        nodes.push_back(std::make_shared<BreakNode>());
    }
    else if (iequals(currentToken.lexical, "repeat"))
    {

        while (canConsume(TokenType::ENDLINE))
        {
            consume(TokenType::ENDLINE);
        }
        std::vector<std::shared_ptr<ASTNode>> whileNodes;
        if (!canConsumeKeyWord("begin"))
        {
            whileNodes.push_back(parseExpression(next(), scope));
        }
        else
        {
            whileNodes.push_back(parseBlock(current(), scope + 1));
        }
        tryConsume(TokenType::SEMICOLON);
        while (canConsume(TokenType::ENDLINE))
        {
            consume(TokenType::ENDLINE);
        }

        consumeKeyWord("until");
        auto expression = parseExpression(next(), scope + 1);
        tryConsume(TokenType::SEMICOLON);

        while (canConsume(TokenType::ENDLINE))
        {
            consume(TokenType::ENDLINE);
        }

        nodes.push_back(std::make_shared<RepeatUntilNode>(expression, whileNodes));
    }
    else
    {
        parseOk = false;
    }
    return parseOk;
}

void Parser::parseFunction(size_t scope, std::vector<std::shared_ptr<ASTNode>> &nodes, bool isProcedure)
{
    consume(TokenType::NAMEDTOKEN);
    auto functionName = std::string(current().lexical);
    m_known_function_names.push_back(functionName);
    bool isExternalFunction = false;
    std::string libName;
    std::string externalName = functionName;

    consume(TokenType::LEFT_CURLY);
    auto token = next();
    std::vector<FunctionArgument> functionParams;
    std::vector<FunctionAttribute> functionAttributes;
    while (token.tokenType != TokenType::RIGHT_CURLY)
    {

        bool isReference = false;
        if (token.tokenType == TokenType::KEYWORD && iequals(token.lexical, "var"))
        {
            next();
            isReference = true;
        }
        token = current();
        const std::string funcParamName = std::string(token.lexical);
        std::vector<std::string> paramNames;
        paramNames.push_back(funcParamName);
        while (canConsume(TokenType::COMMA))
        {
            consume(TokenType::COMMA);
            consume(TokenType::NAMEDTOKEN);
            paramNames.emplace_back(current().lexical);
        }

        consume(TokenType::COLON);
        if (canConsume(TokenType::NAMEDTOKEN))
        {
            token = next();
            auto type = determinVariableTypeByName(std::string(token.lexical));

            for (const auto &param: paramNames)
            {

                if (isVariableDefined(param, scope))
                {
                    m_errors.push_back(
                            ParserError{.file_name = m_file_path.string(),
                                        .token = token,
                                        .message = "A variable with the name " + param + " was allready defined!"});
                }
                else if (!type.has_value())
                {
                    m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                                   .token = token,
                                                   .message = "A type " + std::string(token.lexical) +
                                                              " of the variable " + param +
                                                              " could not be determined!"});
                }
                else
                {

                    m_known_variable_definitions.push_back(
                            VariableDefinition{.variableType = type.value(), .variableName = param, .scopeId = scope});

                    functionParams.push_back(
                            FunctionArgument{.type = type.value(), .argumentName = param, .isReference = isReference});
                }
            }
            tryConsume(TokenType::SEMICOLON);
        }
        else
        {
            // TODO: type def missing
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = token,
                                           .message = "For the parameter definition " + funcParamName +
                                                      " there is a type missing"});
        }

        token = next();
    }
    if (!isProcedure)
        if (!tryConsume(TokenType::COLON))
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = current(),
                                .message = "the return type for the function \"" + functionName + "\" is missing."});
            throw ParserException(m_errors);
        }
    std::shared_ptr<VariableType> returnType;

    if (!isProcedure && consume(TokenType::NAMEDTOKEN))
    {
        const auto typeName = std::string{current().lexical.begin(), current().lexical.end()};
        const auto type = determinVariableTypeByName(typeName);
        if (!type)
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = current(),
                                .message = "A return type " + typeName + " of function could not be determined!"});
        }
        else
        {
            returnType = type.value();
        }

        m_known_variable_definitions.push_back(
                VariableDefinition{.variableType = returnType, .variableName = functionName, .scopeId = scope});
    }
    consume(TokenType::SEMICOLON);

    if (tryConsumeKeyWord("external"))
    {
        isExternalFunction = true;
        tryConsume(TokenType::STRING) || tryConsume(TokenType::CHAR);
        libName = std::string(current().lexical);

        if (tryConsumeKeyWord("name"))
        {
            consume(TokenType::STRING);
            externalName = std::string(current().lexical);
        }
        tryConsume(TokenType::SEMICOLON);
    }
    else if (tryConsumeKeyWord("inline"))
    {
        functionAttributes.emplace_back(FunctionAttribute::Inline);
        consume(TokenType::SEMICOLON);
    }

    tryConsume(TokenType::ENDLINE);

    // parse function body
    if (isExternalFunction)
    {
        nodes.push_back(std::make_shared<FunctionDefinitionNode>(functionName, externalName, libName, functionParams,
                                                                 isProcedure, returnType));
    }
    else
    {
        auto functionBody = parseBlock(current(), scope + 1);
        consume(TokenType::SEMICOLON);
        if (!isProcedure)
        {
            functionBody->addVariableDefinition(VariableDefinition{.variableType = returnType,
                                                                   .variableName = functionName,
                                                                   .scopeId = 0,
                                                                   .value = nullptr,
                                                                   .constant = false});
        }
        auto functionDefinition = std::make_shared<FunctionDefinitionNode>(functionName, functionParams, functionBody,
                                                                           isProcedure, returnType);
        for (auto attribute: functionAttributes)
            functionDefinition->addAttribute(attribute);

        nodes.push_back(functionDefinition);
    }


    for (auto &param: functionParams)
    {
        m_known_variable_definitions.erase(std::ranges::remove_if(m_known_variable_definitions,
                                                                  [param](const VariableDefinition &value)
                                                                  { return param.argumentName == value.variableName; })
                                                   .begin());
    }
}

std::shared_ptr<ASTNode> Parser::parseComparrision(const Token &currentToken, size_t currentScope,
                                                   std::vector<std::shared_ptr<ASTNode>> &nodes)
{

    CMPOperator op = CMPOperator::EQUALS;
    switch (currentToken.tokenType)
    {
        case TokenType::GREATER:
            if (tryConsume(TokenType::EQUAL))
            {
                op = CMPOperator::GREATER_EQUAL;
            }
            else
            {
                op = CMPOperator::GREATER;
            }
            break;
        case TokenType::LESS:
            if (tryConsume(TokenType::EQUAL))
            {
                op = CMPOperator::LESS_EQUAL;
            }
            else
            {
                op = CMPOperator::LESS;
            }
            break;
        case TokenType::EQUAL:
            break;
        default:
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = currentToken,
                                           .message = "unexpected token in comparrision"});
    }
    auto rhs = parseToken(next(), currentScope, {});

    if (nodes.empty())
    {
        m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                       .token = currentToken,
                                       .message = "left hand side of the comparrision is missing"});
        return nullptr;
    }

    return std::make_shared<ComparrisionNode>(op, nodes[0], rhs);
}
std::shared_ptr<ASTNode> Parser::parseExpression(const Token &currentToken, size_t currentScope)
{
    // variant 1
    // TODO: the while loop is broken and useless
    Token token = currentToken;
    std::vector<std::shared_ptr<ASTNode>> nodes;

    while (token.tokenType != TokenType::SEMICOLON && token.tokenType != TokenType::T_EOF)
    {
        switch (token.tokenType)
        {
            case TokenType::ESCAPED_STRING:
            case TokenType::STRING:
            case TokenType::CHAR:
            case TokenType::NUMBER:
            {
                nodes.push_back(parseToken(token, currentScope, {}));

                break;
            }

            case TokenType::NAMEDTOKEN:
            {

                if (canConsume(TokenType::COLON) ||
                    (canConsume(TokenType::LEFT_SQUAR) && canConsume(TokenType::COLON, 4)) ||
                    (canConsume(TokenType::DOT) && canConsume(TokenType::COLON, 3)))
                {
                    parseVariableAssignment(token, currentScope, nodes);
                }
                else
                {
                    auto lhs = parseToken(token, currentScope, {});
                    if (canConsume(TokenType::PLUS) || canConsume(TokenType::MUL) || canConsume(TokenType::MINUS) ||
                        canConsume(TokenType::DIV) || canConsumeKeyWord("mod"))
                    {
                        lhs = parseToken(next(), currentScope, {lhs});
                    }

                    nodes.push_back(lhs);

                    if (!canConsume(TokenType::GREATER) && !canConsume(TokenType::LESS) &&
                        !canConsume(TokenType::EQUAL) && !canConsumeKeyWord("and") && !canConsumeKeyWord("or"))
                        return nodes.back();
                    break;
                }
            }
            break;
            case TokenType::KEYWORD:
            {
                if (!parseKeyWord(currentToken, nodes, currentScope))
                {
                    if (iequals(token.lexical, "then"))
                    {
                        --m_current;
                        return nodes.at(0);
                    }
                    auto op = LogicalOperator::OR;
                    if (token.lexical == "not")
                    {
                        auto rhs = parseExpression(next(), currentScope);
                        return std::make_shared<LogicalExpressionNode>(LogicalOperator::NOT, rhs);
                    }

                    if (token.lexical == "and")
                    {
                        op = LogicalOperator::AND;
                    }
                    else if (token.lexical == "or")
                    {
                        op = LogicalOperator::OR;
                    }
                    else if (iequals(token.lexical, "true"))
                    {
                        return std::make_shared<BooleanNode>(true);
                    }
                    else if (iequals(token.lexical, "false"))
                    {
                        return std::make_shared<BooleanNode>(false);
                    }

                    else
                    {
                        m_current--;
                        if (nodes.empty())
                            return nullptr;
                        return nodes.at(0);
                    }
                    auto lhs = nodes.at(0);
                    auto rhs = parseExpression(next(), currentScope);
                    auto cmp = std::make_shared<LogicalExpressionNode>(op, lhs, rhs);
                    nodes.clear();
                    nodes.push_back(cmp);
                }

                break;
            }
            case TokenType::GREATER:
            case TokenType::LESS:
            case TokenType::EQUAL:
            {
                auto cmp = parseComparrision(token, currentScope, nodes);
                nodes.clear();
                nodes.push_back(cmp);
                break;
            }

            default:
                break;
        }
        token = current();

        if (token.tokenType == TokenType::SEMICOLON || token.tokenType == TokenType::ENDLINE ||
            token.tokenType == TokenType::COMMA || token.tokenType == TokenType::T_EOF ||
            token.tokenType == TokenType::RIGHT_CURLY)
        {
            if (token.tokenType == TokenType::RIGHT_CURLY)
                --m_current;
            break;
        }

        token = next();
    }
    if (nodes.empty())
        return nullptr;
    return nodes.at(0);
}

void Parser::parseVariableAssignment(const Token &currentToken, size_t currentScope,
                                     [[maybe_unused]] std::vector<std::shared_ptr<ASTNode>> &nodes)
{
    auto variableNameToken = TokenWithFile{.token = currentToken, .fileName = m_file_path};

    auto variableName = std::string(currentToken.lexical);

    if (canConsume(TokenType::COLON))
    {
        if (!consume(TokenType::COLON))
        {
        }

        if (!consume(TokenType::EQUAL))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = currentToken,
                                           .message = "missing assignment for varaible!"});
            return;
        }

        if (!isVariableDefined(variableName, currentScope))
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = currentToken,
                                .message = "The variable " + std::string(variableName) + " is not yet declared!"});
            return;
        }

        // parse expression
        auto expression = parseExpression(next(), currentScope);

        nodes.push_back(std::make_shared<VariableAssignmentNode>(variableNameToken, expression));
    }
    else if (canConsume(TokenType::DOT))
    {
        consume(TokenType::DOT);
        if (!consume(TokenType::NAMEDTOKEN))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = current(),
                                           .message = "missing field on variable " + variableName + "!"});
            return;
        }
        auto fieldName = current();
        consume(TokenType::COLON);
        if (!consume(TokenType::EQUAL))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = currentToken,
                                           .message = "missing assignment for varaible!"});
            return;
        }

        if (!isVariableDefined(variableName, currentScope))
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = currentToken,
                                .message = "The variable " + std::string(variableName) + " is not yet declared!"});
            return;
        }

        // parse expression
        auto expression = parseExpression(next(), currentScope);

        auto variable = TokenWithFile{.token = currentToken, .fileName = m_file_path};
        auto field = TokenWithFile{.token = fieldName, .fileName = m_file_path};
        nodes.push_back(std::make_shared<FieldAssignmentNode>(variable, field, expression));
    }
    else
    {
        consume(TokenType::LEFT_SQUAR);
        auto index = parseToken(next(), currentScope, {});
        consume(TokenType::RIGHT_SQUAR);
        consume(TokenType::COLON);
        if (!consume(TokenType::EQUAL))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = currentToken,
                                           .message = "missing assignment for varaible!"});
            return;
        }

        if (!isVariableDefined(variableName, currentScope))
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = currentToken,
                                .message = "The variable " + std::string(variableName) + " is not yet declared!"});
            return;
        }
        auto expression = parseExpression(next(), currentScope);
        nodes.push_back(std::make_shared<ArrayAssignmentNode>(variableNameToken, index, expression));
    }
    tryConsume(TokenType::SEMICOLON);
}

std::unique_ptr<UnitNode> Parser::parseUnit()
{
    try
    {
        std::vector<std::shared_ptr<ASTNode>> nodes;

        Token currentToken = current();

        if (iequals(currentToken.lexical, "program") || iequals(currentToken.lexical, "unit"))
        {
            UnitType unitType = UnitType::UNIT;
            if (iequals(currentToken.lexical, "program"))
                unitType = UnitType::PROGRAM;

            if (consume(TokenType::NAMEDTOKEN))
            {
                int scope = 0;
                auto unitName = std::string(current().lexical);
                consume(TokenType::SEMICOLON);
                while (canConsume(TokenType::ENDLINE))
                {
                    consume(TokenType::ENDLINE);
                }

                if (unitName != "system")
                {
                    importUnit(nodes, "system.pas");
                }


                while (!canConsumeKeyWord("begin") && !canConsumeKeyWord("var") && !canConsumeKeyWord("const"))
                {
                    parseTypeDefinitions(scope);

                    if (canConsume(TokenType::KEYWORD) && !canConsumeKeyWord("begin") && !canConsumeKeyWord("var") &&
                        !canConsumeKeyWord("const"))
                        parseKeyWord(next(), nodes, scope + 1);

                    while (canConsume(TokenType::ENDLINE))
                    {
                        consume(TokenType::ENDLINE);
                    }
                }
                auto block = parseBlock(current(), 0);

                consume(TokenType::DOT);

                std::vector<std::shared_ptr<FunctionDefinitionNode>> functionDefinitions;
                for (auto &node: nodes)
                {
                    if (const auto func = std::dynamic_pointer_cast<FunctionDefinitionNode>(node); func != nullptr)
                    {
                        functionDefinitions.emplace_back(func);
                    }
                }

                return std::make_unique<UnitNode>(unitType, unitName, functionDefinitions, m_typeDefinitions, block);
            }
        }
        m_errors.push_back(
                ParserError{.file_name = m_file_path.string(),
                            .token = m_tokens[m_current + 1],
                            .message = "Unexpected token " + std::string(m_tokens[m_current + 1].lexical) + "!"});
    }
    catch (const ParserException &e)
    {
        std::cerr << e.what();
    }
    return nullptr;
}


std::shared_ptr<ArrayType> Parser::parseArray(size_t scope, bool includeExpressionEnd)
{
    auto isFixedArray = tryConsume(TokenType::LEFT_SQUAR);
    size_t arrayStart = 0;
    size_t arrayEnd = 0;
    if (isFixedArray)
    {

        auto arrayStartNode = parseToken(next(), scope, {});
        if (auto node = std::dynamic_pointer_cast<NumberNode>(arrayStartNode))
        {
            arrayStart = node->getValue();
        }
        consume(TokenType::DOT);
        consume(TokenType::DOT);

        auto arrayEndNode = parseToken(next(), scope, {});
        if (auto node = std::dynamic_pointer_cast<NumberNode>(arrayEndNode))
        {
            arrayEnd = node->getValue();
        }
        consume(TokenType::RIGHT_SQUAR);
    }
    consumeKeyWord("of");
    consume(TokenType::NAMEDTOKEN);
    auto internalTypeName = std::string(current().lexical);
    auto internalType = determinVariableTypeByName(internalTypeName);
    if (includeExpressionEnd)
    {
        consume(TokenType::SEMICOLON);
        tryConsume(TokenType::ENDLINE);
    }

    if (isFixedArray)
    {
        return ArrayType::getFixedArray(arrayStart, arrayEnd, internalType.value());
    }
    else
    {
        return ArrayType::getDynArray(internalType.value());
    }
}

bool Parser::parseTypeDefinitions(int scope)
{
    if (tryConsumeKeyWord("type"))
    {
        while (canConsume(TokenType::ENDLINE))
        {
            consume(TokenType::ENDLINE);
        }

        // parse type definitions
        while (tryConsume(TokenType::NAMEDTOKEN))
        {

            auto typeName = std::string(current().lexical);
            consume(TokenType::EQUAL);
            auto isPointerType = tryConsume(TokenType::CARET); // TODO: build type with the result
            // parse type
            if (tryConsumeKeyWord("array"))
            {
                m_typeDefinitions[typeName] = parseArray(scope);
            }
            else if (tryConsumeKeyWord("record"))
            {
                tryConsume(TokenType::ENDLINE);
                std::vector<VariableDefinition> fieldDefinitions;

                while (!canConsumeKeyWord("end"))
                {
                    auto definitions = parseVariableDefinitions(current(), scope);

                    for (auto &definition: definitions)
                        fieldDefinitions.push_back(definition);
                }

                consumeKeyWord("end");
                consume(TokenType::SEMICOLON);
                tryConsume(TokenType::ENDLINE);

                m_typeDefinitions[typeName] = std::make_shared<RecordType>(fieldDefinitions, typeName);
            }
            else if (tryConsume(TokenType::NAMEDTOKEN))
            {

                auto internalTypeName = std::string(current().lexical);
                auto internalType = determinVariableTypeByName(internalTypeName);
                if (!internalType.has_value())
                {
                    m_errors.push_back(
                            ParserError{.file_name = m_file_path.string(),
                                        .token = current(),
                                        .message = "The type " + internalTypeName + " could not be determined!"});
                    continue;
                }
                if (isPointerType)
                {
                    m_typeDefinitions[typeName] = PointerType::getPointerTo(internalType.value());
                }
                else
                {
                    m_typeDefinitions[typeName] = internalType.value();
                }

                consume(TokenType::SEMICOLON);
                tryConsume(TokenType::ENDLINE);
            }
            while (canConsume(TokenType::ENDLINE))
            {
                consume(TokenType::ENDLINE);
            }
        }
        return true;
    }

    return false;
}

bool Parser::hasError() const { return !m_errors.empty(); }

void Parser::printErrors(std::ostream &outputStream)
{
    for (auto &error: m_errors)
    {
        outputStream << error.file_name << ":" << error.token.row << ":" << error.token.col << ": " << error.message
                     << "\n";
    }
}
