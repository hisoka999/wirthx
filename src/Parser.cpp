#include "Parser.h"
#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <magic_enum/magic_enum.hpp>
#include "Lexer.h"
#include "ast/ArrayAccessNode.h"
#include "ast/ArrayAssignmentNode.h"
#include "ast/BinaryOperationNode.h"
#include "ast/BlockNode.h"
#include "ast/BooleanNode.h"
#include "ast/BreakNode.h"
#include "ast/ComparissionNode.h"
#include "ast/ForNode.h"
#include "ast/FunctionCallNode.h"
#include "ast/FunctionDefinitionNode.h"
#include "ast/IfConditionNode.h"
#include "ast/InputNode.h"
#include "ast/LogicalExpressionNode.h"
#include "ast/NumberNode.h"
#include "ast/PrintNode.h"
#include "ast/ReturnNode.h"
#include "ast/StringConstantNode.h"
#include "ast/SystemFunctionCallNode.h"
#include "ast/VariableAccessNode.h"
#include "ast/VariableAssignmentNode.h"
#include "ast/WhileNode.h"
#include "compare.h"

Parser::Parser(const std::filesystem::path &path, std::vector<Token> &tokens) : m_file_path(path), m_tokens(tokens)
{
    m_typeDefinitions["shortint"] = VariableType::getInteger(8);
    m_typeDefinitions["byte"] = VariableType::getInteger(8);
    m_typeDefinitions["smallint"] = VariableType::getInteger(16);
    m_typeDefinitions["word"] = VariableType::getInteger(16);
    m_typeDefinitions["longint"] = VariableType::getInteger();
    m_typeDefinitions["integer"] = VariableType::getInteger();
    m_typeDefinitions["int64"] = VariableType::getInteger(64);
    m_typeDefinitions["string"] = std::make_shared<VariableType>(VariableBaseType::String, "string");
    m_typeDefinitions["boolean"] = VariableType::getBoolean();
}

Parser::~Parser() {}

bool Parser::hasNext() { return m_current < m_tokens.size() - 1; }

Token &Parser::current() { return m_tokens[m_current]; }

Token &Parser::next()
{
    if (hasNext())
    {
        m_current++;
    }
    return current();
}
bool Parser::canConsume(TokenType tokenType) { return m_tokens[m_current + 1].tokenType == tokenType; }

bool Parser::tryConsume(TokenType tokenType)
{
    if (canConsume(tokenType))
    {
        next();
        return true;
    }
    return false;
}
bool Parser::consume(TokenType tokenType)
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
                                   .message = "expected token  '" + keyword + "' but found " +
                                              std::string(m_tokens[m_current + 1].lexical) + "!"});
    throw ParserException(m_errors);
}

bool Parser::canConsumeKeyWord(const std::string &keyword)
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

std::optional<std::shared_ptr<VariableType>> Parser::determinVariableTypeByName(const std::string &name)
{
    if (m_typeDefinitions.contains(name))
        return m_typeDefinitions.at(name);

    return std::nullopt;
}

std::shared_ptr<ASTNode> Parser::parseToken(const Token &token, size_t currentScope,
                                            std::vector<std::shared_ptr<ASTNode>> nodes)
{
    std::vector<std::shared_ptr<ASTNode>> args;
    switch (token.tokenType)
    {
        case TokenType::NUMBER:
        {
            auto value = std::atoll(token.lexical.data());
            auto lhs = std::make_shared<NumberNode>(value);
            if (canConsume(TokenType::PLUS) || canConsume(TokenType::MINUS))
            {
                auto op = parseToken(next(), currentScope, {lhs});
                return op;
            }
            return lhs;
            // check when ever the next token is a an operator
        }
        case TokenType::STRING:
        {
            return std::make_shared<StringConstantNode>(token.lexical);
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

        case TokenType::NAMEDTOKEN:
        {


            if (canConsume(TokenType::LEFT_CURLY))
            {
                auto functionName = std::string(token.lexical);
                bool isSysCall = isKnownSystemCall(functionName);
                if (!isSysCall && std::find(m_known_function_names.begin(), m_known_function_names.end(),
                                            functionName) == std::end(m_known_function_names))
                {
                    for (auto &fun: m_known_function_names)
                        std::cerr << "func: " << fun << "\n";
                    m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                                   .token = token,
                                                   .message = "a function with the name '" +
                                                              std::string(token.lexical) + "' is not yet defined!"});
                }
                consume(TokenType::LEFT_CURLY);

                Token subToken = next();
                std::vector<std::shared_ptr<ASTNode>> callArgs;
                while (subToken.tokenType != TokenType::RIGHT_CURLY && subToken.tokenType != TokenType::SEMICOLON)
                {
                    switch (subToken.tokenType)
                    {
                        case TokenType::NAMEDTOKEN:
                        {
                            auto node = parseToken(subToken, currentScope, {});

                            callArgs.push_back(node);
                        }
                        break;
                        case TokenType::COMMA:
                            break;
                        default:
                            callArgs.push_back(parseToken(subToken, currentScope, {}));
                    }

                    subToken = next();
                }


                if (isSysCall)
                {
                    return std::make_shared<SystemFunctionCallNode>(functionName, callArgs);
                }
                return std::make_shared<FunctionCallNode>(functionName, callArgs);
            }
            else if (canConsume(TokenType::LEFT_SQUAR))
            {
                Token arrayName = token;
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
            else if (iequals(token.lexical, "false"))
            {
                return std::make_shared<BooleanNode>(false);
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

bool Parser::isVariableDefined(const std::string_view name, size_t scope)
{
    for (const auto &def: m_known_variable_definitions)
    {
        if (iequals(def.variableName, name) && def.scopeId <= scope)
            return true;
    }
    return false;
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
            else if (!type.has_value())
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

            // consume var declarations
            consume(TokenType::NAMEDTOKEN);
            _currentToken = current();
            auto varName = std::string(_currentToken.lexical);

            consume(TokenType::COLON);

            consume(TokenType::NAMEDTOKEN);
            _currentToken = current();
            auto varType = std::string(_currentToken.lexical);
            auto type = determinVariableTypeByName(varType);

            std::shared_ptr<ASTNode> value;
            if (tryConsume(TokenType::EQUAL))
            {
                value = parseToken(next(), scope, {});
            }

            consume(TokenType::SEMICOLON);
            consume(TokenType::ENDLINE);
            if (isVariableDefined(varName, scope))
            {
                m_errors.push_back(
                        ParserError{.file_name = m_file_path.string(),
                                    .token = currentToken,
                                    .message = "A variable with the name " + varName + " was allready defined!"});
                continue;
            }
            else if (!type.has_value())
            {
                m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                               .token = currentToken,
                                               .message = "A type " + varType + " of the variable " + varName +
                                                          " could not be determined!"});
                continue;
            }

            m_known_variable_definitions.push_back(VariableDefinition{
                    .variableType = type.value(), .variableName = varName, .scopeId = scope, .value = value});
            blockVariableDefinitions.push_back(VariableDefinition{
                    .variableType = type.value(), .variableName = varName, .scopeId = scope, .value = value});
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

bool Parser::parseKeyWord(const Token &currentToken, std::vector<std::shared_ptr<ASTNode>> &nodes, size_t scope)
{
    bool parseOk = true;
    if (currentToken.lexical == "if")
    {
        auto conditionNode = parseExpression(next(), scope);
        tryConsumeKeyWord("then");
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
        consume(TokenType::NAMEDTOKEN);
        auto functionName = std::string(current().lexical);
        m_known_function_names.push_back(functionName);

        consume(TokenType::LEFT_CURLY);
        auto token = next();
        std::vector<FunctionArgument> functionParams;
        while (token.tokenType != TokenType::RIGHT_CURLY)
        {

            const auto isReference = tryConsumeKeyWord("var");
            const std::string paramName = {token.lexical.begin(), token.lexical.end()};

            consume(TokenType::COLON);
            if (canConsume(TokenType::NAMEDTOKEN))
            {
                token = next();
                auto type = determinVariableTypeByName(std::string(token.lexical));
                if (isVariableDefined(paramName, scope))
                {
                    m_errors.push_back(
                            ParserError{.file_name = m_file_path.string(),
                                        .token = currentToken,
                                        .message = "A variable with the name " + paramName + " was allready defined!"});
                }
                else if (!type.has_value())
                {
                    m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                                   .token = currentToken,
                                                   .message = "A type " + std::string(token.lexical) +
                                                              " of the variable " + paramName +
                                                              " could not be determined!"});
                }

                m_known_variable_definitions.push_back(
                        VariableDefinition{.variableType = type.value(), .variableName = paramName, .scopeId = scope});

                functionParams.push_back(
                        FunctionArgument{.type = type.value(), .argumentName = paramName, .isReference = isReference});

                tryConsume(TokenType::SEMICOLON);
            }
            else
            {
                // TODO type def missing
                m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                               .token = currentToken,
                                               .message = "For the parameter definition " + paramName +
                                                          " there is a type missing"});
            }

            token = next();
        }
        consume(TokenType::SEMICOLON);
        tryConsume(TokenType::ENDLINE);

        std::vector<std::shared_ptr<ASTNode>> functionBody;
        // parse function body

        auto body = parseBlock(current(), scope + 1);
        consume(TokenType::SEMICOLON);

        nodes.push_back(std::make_shared<FunctionDefinitionNode>(functionName, functionParams, body, true));
    }
    else if (iequals(currentToken.lexical, "function"))
    {
        parseFunction(scope, nodes);
    }
    else if (iequals(currentToken.lexical, "uses"))
    {
        if (consume(TokenType::NAMEDTOKEN))
        {
            auto filename = std::string(current().lexical) + ".pas";
            auto path = this->m_file_path.parent_path() / filename;
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

                return false;
            }
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            std::string buffer(size, ' ');
            file.seekg(0);
            file.read(&buffer[0], size);

            auto tokens = lexer.tokenize(std::string_view{buffer});
            Parser parser(path, tokens);
            auto unit = parser.parseUnit();

            for (auto &node: unit->getFunctionDefinitions())
            {
                nodes.push_back(node);
            }
            for (auto &name: parser.m_known_function_names)
            {
                m_known_function_names.push_back(name);
            }

            for (auto &error: parser.m_errors)
            {
                m_errors.push_back(error);
            }
        }
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
        whileNodes.push_back(parseBlock(current(), 0));

        nodes.push_back(std::make_shared<WhileNode>(expression, whileNodes));
    }
    else if (iequals(currentToken.lexical, "for"))
    {
        consume(TokenType::NAMEDTOKEN);
        auto loopVariable = std::string(current().lexical);
        consume(TokenType::COLON);
        consume(TokenType::EQUAL);
        auto loopStart = parseToken(next(), 0, nodes);

        consumeKeyWord("to");
        auto loopEnd = parseToken(next(), 0, nodes);

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
        whileNodes.push_back(parseBlock(current(), 0));

        nodes.push_back(std::make_shared<ForNode>(loopVariable, loopStart, loopEnd, whileNodes));
    }
    else if (iequals(currentToken.lexical, "break"))
    {
        nodes.push_back(std::make_shared<BreakNode>());
    }
    else
    {
        parseOk = false;
    }
    return parseOk;
}

void Parser::parseFunction(size_t scope, std::vector<std::shared_ptr<ASTNode>> &nodes)
{
    consume(TokenType::NAMEDTOKEN);
    auto functionName = std::string(current().lexical);
    m_known_function_names.push_back(functionName);

    consume(TokenType::LEFT_CURLY);
    auto token = next();
    std::vector<FunctionArgument> functionParams;
    while (token.tokenType != TokenType::RIGHT_CURLY)
    {

        const auto isReference = tryConsumeKeyWord("var");
        const std::string funcParamName = std::string(token.lexical);

        consume(TokenType::COLON);
        if (canConsume(TokenType::NAMEDTOKEN))
        {
            token = next();
            auto type = determinVariableTypeByName(std::string(token.lexical));
            if (isVariableDefined(funcParamName, scope))
            {
                m_errors.push_back(
                        ParserError{.file_name = m_file_path.string(),
                                    .token = token,
                                    .message = "A variable with the name " + funcParamName + " was allready defined!"});
            }
            else if (!type.has_value())
            {
                m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                               .token = token,
                                               .message = "A type " + std::string(token.lexical) + " of the variable " +
                                                          funcParamName + " could not be determined!"});
            }
            else
            {
                m_known_variable_definitions.push_back(VariableDefinition{
                        .variableType = type.value(), .variableName = funcParamName, .scopeId = scope});

                functionParams.push_back(FunctionArgument{
                        .type = type.value(), .argumentName = funcParamName, .isReference = isReference});
            }

            tryConsume(TokenType::SEMICOLON);
        }
        else
        {
            // TODO type def missing
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = token,
                                           .message = "For the parameter definition " + funcParamName +
                                                      " there is a type missing"});
        }

        token = next();
    }
    consume(TokenType::COLON);
    std::shared_ptr<VariableType> returnType;
    if (consume(TokenType::NAMEDTOKEN))
    {
        auto typeName = std::string{current().lexical.begin(), current().lexical.end()};
        auto type = determinVariableTypeByName(typeName);
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
    tryConsume(TokenType::ENDLINE);

    // parse function body

    auto functionBody = parseBlock(current(), scope + 1);
    consume(TokenType::SEMICOLON);

    nodes.push_back(
            std::make_shared<FunctionDefinitionNode>(functionName, functionParams, functionBody, false, returnType));
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

    Token token = currentToken;
    std::vector<std::shared_ptr<ASTNode>> nodes;

    while (token.tokenType != TokenType::SEMICOLON && token.tokenType != TokenType::T_EOF)
    {
        switch (token.tokenType)
        {
            case TokenType::STRING:
            case TokenType::NUMBER:
                nodes.push_back(parseToken(token, currentScope, {}));
                break;
            case TokenType::NAMEDTOKEN:
            {
                if (canConsume(TokenType::COLON) || canConsume(TokenType::LEFT_SQUAR))
                {
                    parseVariableAssignment(token, currentScope, nodes);
                }
                else
                {
                    auto lhs = parseToken(token, currentScope, {});
                    if (canConsume(TokenType::PLUS) || canConsume(TokenType::MUL) || canConsume(TokenType::MINUS))
                    {
                        lhs = parseToken(next(), currentScope, {lhs});
                    }

                    nodes.push_back(lhs);
                }
            }
            break;
            case TokenType::KEYWORD:
            {
                if (!parseKeyWord(currentToken, nodes, currentScope))
                {
                    if (iequals(token.lexical, "then"))
                    {
                        break;
                    }
                    LogicalOperator op = LogicalOperator::OR;
                    if (token.lexical == "not")
                    {
                        auto rhs = parseExpression(next(), currentScope);
                        return std::make_shared<LogicalExpressionNode>(LogicalOperator::NOT, rhs);
                    }
                    else if (token.lexical == "and")
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
            token.tokenType == TokenType::T_EOF || token.tokenType == TokenType::RIGHT_CURLY)
            break;

        token = next();
    }
    if (nodes.empty())
        return nullptr;
    return nodes.at(0);
}

void Parser::parseVariableAssignment(const Token &currentToken, size_t currentScope,
                                     [[maybe_unused]] std::vector<std::shared_ptr<ASTNode>> &nodes)
{
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

        nodes.push_back(std::make_shared<VariableAssignmentNode>(variableName, expression));
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
        nodes.push_back(std::make_shared<ArrayAssignmentNode>(variableName, index, expression));
    }
}

std::unique_ptr<UnitNode> Parser::parseUnit()
{
    try
    {
        std::vector<std::shared_ptr<ASTNode>> nodes;

        Token currentToken = current();
        int scope = 0;

        if (iequals(currentToken.lexical, "program") || iequals(currentToken.lexical, "unit"))
        {
            UnitType unitType = UnitType::UNIT;
            if (iequals(currentToken.lexical, "program"))
                unitType = UnitType::PROGRAM;

            if (consume(TokenType::NAMEDTOKEN))
            {
                auto functionName = std::string(current().lexical);
                consume(TokenType::SEMICOLON);
                while (canConsume(TokenType::ENDLINE))
                {
                    consume(TokenType::ENDLINE);
                }

                parseTypeDefinitions(scope);

                while (!canConsumeKeyWord("begin") && !canConsumeKeyWord("var") && !canConsumeKeyWord("const"))
                {
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
                    auto func = std::dynamic_pointer_cast<FunctionDefinitionNode>(node);
                    if (func != nullptr)
                    {
                        functionDefinitions.emplace_back(func);
                    }
                }

                return std::make_unique<UnitNode>(unitType, functionName, functionDefinitions, m_typeDefinitions,
                                                  block);
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

void Parser::parseTypeDefinitions(int scope)
{
    if (tryConsumeKeyWord("type"))
    {
        while (canConsume(TokenType::ENDLINE))
        {
            consume(TokenType::ENDLINE);
        }
        next();
        // parse type definitions
        while (current().tokenType == TokenType::NAMEDTOKEN)
        {

            auto typeName = std::string(current().lexical);
            consume(TokenType::EQUAL);
            // parse type
            if (tryConsumeKeyWord("array"))
            {
                consume(TokenType::LEFT_SQUAR);
                size_t arrayStart = 0;
                auto arrayStartNode = parseToken(next(), scope, {});
                if (auto node = std::dynamic_pointer_cast<NumberNode>(arrayStartNode))
                {
                    arrayStart = node->getValue();
                }
                consume(TokenType::DOT);
                consume(TokenType::DOT);
                size_t arrayEnd = 0;
                auto arrayEndNode = parseToken(next(), scope, {});
                if (auto node = std::dynamic_pointer_cast<NumberNode>(arrayEndNode))
                {
                    arrayEnd = node->getValue();
                }
                consume(TokenType::RIGHT_SQUAR);
                consumeKeyWord("of");
                consume(TokenType::NAMEDTOKEN);
                auto internalTypeName = std::string(current().lexical);
                auto internalType = determinVariableTypeByName(internalTypeName);
                consume(TokenType::SEMICOLON);
                tryConsume(TokenType::ENDLINE);
                m_typeDefinitions[typeName] = ArrayType::getArray(arrayStart, arrayEnd, internalType.value());
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
                m_typeDefinitions[typeName] = internalType.value();
                consume(TokenType::SEMICOLON);
                tryConsume(TokenType::ENDLINE);
            }
        }
    }
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
