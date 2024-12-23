#include "Parser.h"

#include <cmath>
#include <fstream>
#include <iostream>

#include "ast/ArrayAccessNode.h"
#include "ast/ArrayAssignmentNode.h"
#include "ast/BinaryOperationNode.h"
#include "ast/BooleanNode.h"
#include "ast/BreakNode.h"
#include "ast/CharConstantNode.h"
#include "ast/ComparissionNode.h"
#include "ast/FieldAccessNode.h"
#include "ast/FieldAssignmentNode.h"
#include "ast/ForNode.h"
#include "ast/FunctionCallNode.h"
#include "ast/IfConditionNode.h"
#include "ast/LogicalExpressionNode.h"
#include "ast/NumberNode.h"
#include "ast/RecordType.h"
#include "ast/RepeatUntilNode.h"
#include "ast/StringConstantNode.h"
#include "ast/SystemFunctionCallNode.h"
#include "ast/VariableAccessNode.h"
#include "ast/VariableAssignmentNode.h"
#include "ast/WhileNode.h"
#include "compare.h"
#include "magic_enum/magic_enum.hpp"


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
bool Parser::hasError() const { return !m_errors.empty(); }
void Parser::printErrors(std::ostream &outputStream)
{
    for (auto &error: m_errors)
    {
        outputStream << error.file_name << ":" << error.token.row << ":" << error.token.col << ": " << error.message
                     << "\n";
    }
}


Token Parser::next()
{
    ++m_current;
    return current();
}
Token Parser::current() { return m_tokens[m_current]; }
bool Parser::hasNext() const { return m_current < m_tokens.size(); }
bool Parser::consume(const TokenType tokenType)
{
    if (canConsume(tokenType))
    {
        next();
        return true;
    }

    m_errors.push_back(ParserError{
            .file_name = m_file_path.string(),
            .token = m_tokens[m_current + 1],
            .message = "expected token  '" + std::string(magic_enum::enum_name(tokenType)) + "' but found " +
                       std::string(magic_enum::enum_name(m_tokens[m_current + 1].tokenType)) + "!"});
    throw ParserException(m_errors);

    return false;
}
bool Parser::tryConsume(const TokenType tokenType)
{
    if (canConsume(tokenType))
    {
        next();
        return true;
    }
    return false;
}
bool Parser::canConsume(TokenType tokenType) const { return canConsume(tokenType, 1); }
bool Parser::canConsume(TokenType tokenType, size_t next) const
{
    return hasNext() && m_tokens[m_current + next].tokenType == tokenType;
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

std::shared_ptr<ASTNode> Parser::parseNumber()
{
    consume(TokenType::NUMBER);
    auto token = current();
    auto value = std::atoll(token.lexical.data());
    auto base = 1 + static_cast<int>(std::log2(value));
    base = (base > 32) ? 64 : 32;
    return std::make_shared<NumberNode>(value, base);
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

void Parser::parseTypeDefinitions(const size_t scope)
{


    // parse type definitions
    while (tryConsume(TokenType::NAMEDTOKEN))
    {

        auto typeName = std::string(current().lexical);
        consume(TokenType::EQUAL);
        auto isPointerType = tryConsume(TokenType::CARET);
        // parse type
        if (tryConsumeKeyWord("array"))
        {
            m_typeDefinitions[typeName] = parseArray(scope);

            consume(TokenType::SEMICOLON);
        }
        else if (tryConsumeKeyWord("record"))
        {
            std::vector<VariableDefinition> fieldDefinitions;

            while (!canConsumeKeyWord("end"))
            {
                auto definitions = parseVariableDefinitions(scope);

                for (auto &definition: definitions)
                    fieldDefinitions.push_back(definition);
            }

            consumeKeyWord("end");
            consume(TokenType::SEMICOLON);


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
        }
    }
}
std::shared_ptr<ArrayType> Parser::parseArray(size_t scope)
{
    auto isFixedArray = tryConsume(TokenType::LEFT_SQUAR);
    size_t arrayStart = 0;
    size_t arrayEnd = 0;
    if (isFixedArray)
    {

        auto arrayStartNode = parseToken(scope);
        if (auto node = std::dynamic_pointer_cast<NumberNode>(arrayStartNode))
        {
            arrayStart = node->getValue();
        }
        consume(TokenType::DOT);
        consume(TokenType::DOT);

        auto arrayEndNode = parseToken(scope);
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


    if (isFixedArray)
    {
        return ArrayType::getFixedArray(arrayStart, arrayEnd, internalType.value());
    }
    else
    {
        return ArrayType::getDynArray(internalType.value());
    }
}
std::optional<VariableDefinition> Parser::parseConstantDefinition(size_t scope)
{

    // consume var declarations
    consume(TokenType::NAMEDTOKEN);
    Token varNameToken = current();

    auto varName = std::string(current().lexical);


    std::optional<std::shared_ptr<VariableType>> type;
    std::string varType;

    if (tryConsume(TokenType::COLON))
    {
        consume(TokenType::NAMEDTOKEN);
        varType = std::string(current().lexical);
        type = determinVariableTypeByName(varType);
    }
    std::shared_ptr<ASTNode> value;
    if (consume(TokenType::EQUAL))
    {
        value = parseToken(scope);

        // determin the type from the parsed token
        if (!type)
        {
            type = value->resolveType(nullptr, nullptr);
            if (type.has_value())
                varType = type.value()->typeName;
        }
    }

    consume(TokenType::SEMICOLON);

    if (isVariableDefined(varName, scope))
    {
        m_errors.push_back(
                ParserError{.file_name = m_file_path.string(),
                            .token = varNameToken,
                            .message = "A variable or constant with the name " + varName + " was allready defined!"});
        return std::nullopt;
    }

    if (!type.has_value())
    {
        m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                       .token = varNameToken,
                                       .message = "A type " + varType + " of the variable " + varName +
                                                  " could not be determined!"});
        return std::nullopt;
    }

    return VariableDefinition{
            .variableType = type.value(), .variableName = varName, .scopeId = scope, .value = value, .constant = true};
}
std::vector<VariableDefinition> Parser::parseVariableDefinitions(const size_t scope)
{
    std::vector<VariableDefinition> result;
    Token _currentToken;

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
            type = parseArray(scope);
        }
    }
    std::shared_ptr<ASTNode> value;
    if (tryConsume(TokenType::EQUAL))
    {
        value = parseToken(scope);

        // determin the type from the parsed token
        if (!type)
        {
            type = value->resolveType(nullptr, nullptr);
            if (type.has_value())
                varType = type.value()->typeName;
        }
    }

    consume(TokenType::SEMICOLON);
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
        if (!type.has_value())
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

std::shared_ptr<ASTNode> Parser::parseLogicalExpression(const size_t scope, std::shared_ptr<ASTNode> lhs)
{
    if (canConsumeKeyWord("not"))
    {
        consumeKeyWord("not");
        auto rhs = parseExpression(scope);
        return parseExpression(scope, std::make_shared<LogicalExpressionNode>(LogicalOperator::NOT, rhs));
    }

    if (!lhs)
        return nullptr;

    if (canConsumeKeyWord("or"))
    {
        consumeKeyWord("or");
        auto rhs = parseExpression(scope);
        return parseExpression(scope, std::make_shared<LogicalExpressionNode>(LogicalOperator::OR, lhs, rhs));
    }
    if (canConsumeKeyWord("and"))
    {
        consumeKeyWord("and");
        auto rhs = parseExpression(scope);
        return parseExpression(scope, std::make_shared<LogicalExpressionNode>(LogicalOperator::AND, lhs, rhs));
    }
    return lhs;
}


std::shared_ptr<ASTNode> Parser::parseBaseExpression(const size_t scope, const std::shared_ptr<ASTNode> &origLhs)
{
    auto lhs = (origLhs) ? origLhs : parseToken(scope);

    if (tryConsume(TokenType::PLUS))
    {
        auto rhs = parseBaseExpression(scope);
        return parseExpression(scope, std::make_shared<BinaryOperationNode>(Operator::PLUS, lhs, rhs));
    }
    if (tryConsume(TokenType::MINUS))
    {
        auto rhs = parseBaseExpression(scope);
        return parseExpression(scope, std::make_shared<BinaryOperationNode>(Operator::MINUS, lhs, rhs));
    }
    if (tryConsume(TokenType::MUL))
    {
        auto rhs = parseBaseExpression(scope);
        return parseExpression(scope, std::make_shared<BinaryOperationNode>(Operator::MUL, lhs, rhs));
    }
    if (tryConsume(TokenType::DIV))
    {
        auto rhs = parseBaseExpression(scope);
        return parseExpression(scope, std::make_shared<BinaryOperationNode>(Operator::DIV, lhs, rhs));
    }
    if (canConsumeKeyWord("mod"))
    {
        consumeKeyWord("mod");
        auto rhs = parseToken(scope);
        return parseExpression(scope, std::make_shared<BinaryOperationNode>(Operator::MOD, lhs, rhs));
    }


    if (canConsume(TokenType::GREATER))
    {
        consume(TokenType::GREATER);
        if (canConsume(TokenType::EQUAL))
        {
            consume(TokenType::EQUAL);
            auto rhs = parseBaseExpression(scope);
            return std::make_shared<ComparrisionNode>(CMPOperator::GREATER_EQUAL, lhs, rhs);
        }
        auto rhs = parseBaseExpression(scope);
        return parseExpression(scope, std::make_shared<ComparrisionNode>(CMPOperator::GREATER, lhs, rhs));
    }

    if (canConsume(TokenType::LESS))
    {
        consume(TokenType::LESS);
        if (canConsume(TokenType::EQUAL))
        {
            consume(TokenType::EQUAL);
            auto rhs = parseBaseExpression(scope);
            return std::make_shared<ComparrisionNode>(CMPOperator::LESS_EQUAL, lhs, rhs);
        }
        auto rhs = parseBaseExpression(scope);
        return parseExpression(scope, std::make_shared<ComparrisionNode>(CMPOperator::LESS, lhs, rhs));
    }

    if (canConsume(TokenType::EQUAL))
    {
        consume(TokenType::EQUAL);

        auto rhs = parseBaseExpression(scope);
        return parseExpression(scope, std::make_shared<ComparrisionNode>(CMPOperator::EQUALS, lhs, rhs));
    }

    return lhs;
}

std::shared_ptr<ASTNode> Parser::parseExpression(const size_t scope, const std::shared_ptr<ASTNode> &origLhs)
{
    auto lhs = parseLogicalExpression(scope, origLhs);
    if (!lhs)
        lhs = origLhs;

    if (!lhs)
        lhs = parseBaseExpression(scope);

    if (auto rhs = parseLogicalExpression(scope, parseBaseExpression(scope, lhs)))
        return rhs;

    return lhs;
}

std::shared_ptr<ASTNode> Parser::parseVariableAssignment(size_t scope)
{
    consume(TokenType::NAMEDTOKEN);
    auto currentToken = current();
    auto variableNameToken = TokenWithFile{.token = current(), .fileName = m_file_path};

    auto variableName = std::string(current().lexical);

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
            return nullptr;
        }

        if (!isVariableDefined(variableName, scope))
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = currentToken,
                                .message = "The variable " + std::string(variableName) + " is not yet declared!"});
            return nullptr;
        }

        // parse expression
        auto expression = parseExpression(scope);
        return std::make_shared<VariableAssignmentNode>(variableNameToken, expression);
    }
    else if (canConsume(TokenType::DOT))
    {
        consume(TokenType::DOT);
        if (!consume(TokenType::NAMEDTOKEN))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = current(),
                                           .message = "missing field on variable " + variableName + "!"});
            return nullptr;
        }
        auto fieldName = current();
        consume(TokenType::COLON);
        if (!consume(TokenType::EQUAL))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = currentToken,
                                           .message = "missing assignment for varaible!"});
            return nullptr;
        }

        if (!isVariableDefined(variableName, scope))
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = currentToken,
                                .message = "The variable " + std::string(variableName) + " is not yet declared!"});
            return nullptr;
        }

        // parse expression
        auto expression = parseExpression(scope);

        auto variable = TokenWithFile{.token = currentToken, .fileName = m_file_path};
        auto field = TokenWithFile{.token = fieldName, .fileName = m_file_path};
        return std::make_shared<FieldAssignmentNode>(variable, field, expression);
    }
    else
    {
        consume(TokenType::LEFT_SQUAR);
        auto index = parseExpression(scope);
        consume(TokenType::RIGHT_SQUAR);
        consume(TokenType::COLON);
        if (!consume(TokenType::EQUAL))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = currentToken,
                                           .message = "missing assignment for varaible!"});
            return nullptr;
        }

        if (!isVariableDefined(variableName, scope))
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = currentToken,
                                .message = "The variable " + std::string(variableName) + " is not yet declared!"});
            return nullptr;
        }
        auto expression = parseExpression(scope);
        return std::make_shared<ArrayAssignmentNode>(variableNameToken, index, expression);
    }
}
std::shared_ptr<ASTNode> Parser::parseVariableAccess(size_t scope)
{
    consume(TokenType::NAMEDTOKEN);
    auto token = current();
    auto variableName = std::string(current().lexical);
    if (canConsume(TokenType::LEFT_SQUAR))
    {
        const Token arrayName = token;
        if (!isVariableDefined(token.lexical, scope))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = token,
                                           .message = "A variable with the name '" + std::string(token.lexical) +
                                                      "' is not yet defined!"});
            return nullptr;
        }
        consume(TokenType::LEFT_SQUAR);
        auto indexNode = parseToken(scope);
        consume(TokenType::RIGHT_SQUAR);
        return std::make_shared<ArrayAccessNode>(TokenWithFile{.token = arrayName, .fileName = m_file_path}, indexNode);
    }
    if (canConsume(TokenType::DOT))
    {
        consume(TokenType::DOT);
        consume(TokenType::NAMEDTOKEN);
        Token field = current();
        if (!isVariableDefined(token.lexical, scope))
        {
            m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                           .token = token,
                                           .message = "A variable with the name '" + std::string(token.lexical) +
                                                      "' is not yet defined!"});
            return nullptr;
        }
        return std::make_shared<FieldAccessNode>(TokenWithFile{.token = token, .fileName = m_file_path},
                                                 TokenWithFile{.token = field, .fileName = m_file_path}

        );
    }

    if (!isVariableDefined(token.lexical, scope))
    {
        m_errors.push_back(ParserError{.file_name = m_file_path.string(),
                                       .token = token,
                                       .message = "A variable with the name '" + std::string(token.lexical) +
                                                  "' is not yet defined!"});
        return nullptr;
    }
    return std::make_shared<VariableAccessNode>(token.lexical);
}
std::shared_ptr<ASTNode> Parser::parseToken(size_t scope)
{
    if (canConsume(TokenType::NUMBER))
    {
        return parseNumber();
    }
    if (canConsume(TokenType::STRING))
    {
        consume(TokenType::STRING);

        return std::make_shared<StringConstantNode>(std::string(current().lexical));
    }
    if (canConsume(TokenType::CHAR))
    {
        consume(TokenType::CHAR);
        return std::make_shared<CharConstantNode>(std::string(current().lexical));
    }
    if (canConsume(TokenType::ESCAPED_STRING))
    {
        consume(TokenType::ESCAPED_STRING);
        return parseEscapedString(current());
    }
    if (canConsume(TokenType::NAMEDTOKEN))
    {
        if (canConsume(TokenType::LEFT_CURLY, 2))
        {
            return parseFunctionCall(scope);
        }

        return parseVariableAccess(scope);
    }
    if (tryConsumeKeyWord("true"))
    {
        return std::make_shared<BooleanNode>(true);
    }
    if (tryConsumeKeyWord("false"))
    {
        return std::make_shared<BooleanNode>(false);
    }
    return nullptr;
}
std::shared_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition(size_t scope, bool isFunction)
{
    consume(TokenType::NAMEDTOKEN);
    auto functionName = std::string(current().lexical);
    m_known_function_names.push_back(functionName);
    bool isExternalFunction = false;
    std::string libName;
    std::string externalName = functionName;

    std::shared_ptr<FunctionDefinitionNode> functionDefinition = nullptr;

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
    if (isFunction)
        if (!tryConsume(TokenType::COLON))
        {
            m_errors.push_back(
                    ParserError{.file_name = m_file_path.string(),
                                .token = current(),
                                .message = "the return type for the function \"" + functionName + "\" is missing."});
            throw ParserException(m_errors);
        }
    std::shared_ptr<VariableType> returnType;

    if (isFunction && consume(TokenType::NAMEDTOKEN))
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


    // parse function body
    if (isExternalFunction)
    {
        functionDefinition = std::make_shared<FunctionDefinitionNode>(functionName, externalName, libName,
                                                                      functionParams, !isFunction, returnType);
    }
    else
    {
        auto functionBody = parseBlock(scope + 1);
        consume(TokenType::SEMICOLON);
        if (isFunction)
        {
            functionBody->addVariableDefinition(VariableDefinition{.variableType = returnType,
                                                                   .variableName = functionName,
                                                                   .scopeId = 0,
                                                                   .value = nullptr,
                                                                   .constant = false});
        }
        functionDefinition = std::make_shared<FunctionDefinitionNode>(functionName, functionParams, functionBody,
                                                                      !isFunction, returnType);
        for (auto attribute: functionAttributes)
            functionDefinition->addAttribute(attribute);
    }


    for (auto &param: functionParams)
    {
        m_known_variable_definitions.erase(std::ranges::remove_if(m_known_variable_definitions,
                                                                  [param](const VariableDefinition &value)
                                                                  { return param.argumentName == value.variableName; })
                                                   .begin());
    }
    return functionDefinition;
}

std::shared_ptr<ASTNode> Parser::parseStatement(size_t scope)
{
    std::shared_ptr<ASTNode> result = nullptr;
    if (canConsume(TokenType::NAMEDTOKEN))
    {
        if (canConsume(TokenType::LEFT_CURLY, 2))
        {
            result = parseFunctionCall(scope);
        }
        else
        {
            result = parseVariableAssignment(scope);
        }
        consume(TokenType::SEMICOLON);
    }
    else if (canConsume(TokenType::KEYWORD))
    {
        result = parseKeyword(scope);
    }
    if (!result)
    {
        m_errors.push_back(
                ParserError{.file_name = m_file_path.string(),
                            .token = m_tokens[m_current + 1],
                            .message = "unexpected token found " +
                                       std::string(magic_enum::enum_name(m_tokens[m_current + 1].tokenType)) + "!"});
    }

    return result;
}


std::shared_ptr<BlockNode> Parser::parseBlock(size_t scope)
{
    std::vector<VariableDefinition> variable_definitions;

    if (tryConsumeKeyWord("const"))
    {
        while (!canConsumeKeyWord("begin") && !canConsumeKeyWord("var"))
        {
            auto definition = parseConstantDefinition(scope);
            if (definition.has_value())
            {
                variable_definitions.push_back(definition.value());
                m_known_variable_definitions.push_back(definition.value());
            }
        }
    }

    if (tryConsumeKeyWord("var"))
    {
        while (!canConsumeKeyWord("begin"))
        {
            auto def = parseVariableDefinitions(scope);
            for (auto &definition: def)
            {
                variable_definitions.push_back(definition);
                m_known_variable_definitions.push_back(definition);
            }
        }
    }
    consumeKeyWord("begin");
    std::vector<std::shared_ptr<ASTNode>> expressions;
    while (!tryConsumeKeyWord("end"))
    {
        if (auto statement = parseStatement(scope))
        {
            expressions.push_back(statement);
        }
        else if (hasError())
        {
            throw ParserException(m_errors);
        }
        else
        {
            assert(false && "could not parse statement");
        }
    }

    return std::make_shared<BlockNode>(variable_definitions, expressions);
}
std::shared_ptr<ASTNode> Parser::parseKeyword(size_t scope)
{
    if (tryConsumeKeyWord("if"))
    {
        auto condition = parseExpression(scope);
        std::vector<std::shared_ptr<ASTNode>> ifStatements;
        std::vector<std::shared_ptr<ASTNode>> elseStatements;
        consumeKeyWord("then");
        if (canConsumeKeyWord("begin"))
        {
            ifStatements.emplace_back(parseBlock(scope + 1));
            tryConsume(TokenType::SEMICOLON);
        }
        else
        {
            ifStatements.emplace_back(parseStatement(scope));
        }
        if (tryConsumeKeyWord("else"))
        {
            if (canConsumeKeyWord("begin"))
            {
                elseStatements.emplace_back(parseBlock(scope + 1));
                tryConsume(TokenType::SEMICOLON);
            }
            else
            {
                elseStatements.emplace_back(parseStatement(scope));
            }
        }

        return std::make_shared<IfConditionNode>(condition, ifStatements, elseStatements);
    }

    if (tryConsumeKeyWord("for"))
    {
        consume(TokenType::NAMEDTOKEN);
        auto loopVariable = std::string(current().lexical);
        m_known_variable_definitions.push_back(VariableDefinition{
                .variableType = VariableType::getInteger(64), .variableName = loopVariable, .scopeId = scope + 1});
        consume(TokenType::COLON);
        consume(TokenType::EQUAL);
        auto loopStart = parseToken(scope + 1);

        consumeKeyWord("to");
        auto loopEnd = parseToken(scope + 1);

        std::vector<std::shared_ptr<ASTNode>> forNodes;

        consumeKeyWord("do");

        if (canConsumeKeyWord("begin"))
        {
            forNodes.emplace_back(parseBlock(scope + 1));
            tryConsume(TokenType::SEMICOLON);
        }
        else
        {
            forNodes.emplace_back(parseStatement(scope));
        }


        return std::make_shared<ForNode>(loopVariable, loopStart, loopEnd, forNodes);
    }
    if (tryConsumeKeyWord("while"))
    {
        auto expression = parseExpression(scope + 1);
        std::vector<std::shared_ptr<ASTNode>> whileNodes;

        consumeKeyWord("do");
        if (!canConsumeKeyWord("begin"))
        {
            whileNodes.push_back(parseStatement(scope));
            tryConsume(TokenType::SEMICOLON);
        }
        else
        {
            whileNodes.push_back(parseBlock(scope + 1));
            consume(TokenType::SEMICOLON);
        }

        return std::make_shared<WhileNode>(expression, whileNodes);
    }

    if (tryConsumeKeyWord("repeat"))
    {
        std::vector<std::shared_ptr<ASTNode>> whileNodes;
        if (!canConsumeKeyWord("begin"))
        {
            whileNodes.push_back(parseStatement(scope));
        }
        else
        {
            whileNodes.push_back(parseBlock(scope + 1));
        }
        tryConsume(TokenType::SEMICOLON);

        consumeKeyWord("until");
        auto expression = parseExpression(scope + 1);
        tryConsume(TokenType::SEMICOLON);

        return std::make_shared<RepeatUntilNode>(expression, whileNodes);
    }

    if (tryConsumeKeyWord("break"))
    {
        tryConsume(TokenType::SEMICOLON);
        return std::make_shared<BreakNode>();
    }

    m_errors.push_back(
            ParserError{.file_name = m_file_path.string(),
                        .token = m_tokens[m_current + 1],
                        .message = "unexpected keyword found " + std::string(m_tokens[m_current + 1].lexical) + "!"});

    return nullptr;
}
std::shared_ptr<ASTNode> Parser::parseFunctionCall(size_t scope)
{
    consume(TokenType::NAMEDTOKEN);
    // auto nameToken = current();
    auto functionName = std::string(current().lexical);
    const bool isSysCall = isKnownSystemCall(functionName);
    if (!isSysCall && std::ranges::find(m_known_function_names, functionName) == std::end(m_known_function_names))
    {
        for (auto &fun: m_known_function_names)
            std::cerr << "func: " << fun << "\n";
        m_errors.push_back(
                ParserError{.file_name = m_file_path.string(),
                            .token = current(),
                            .message = "a function with the name '" + functionName + "' is not yet defined!"});
    }

    std::vector<std::shared_ptr<ASTNode>> callArgs;
    consume(TokenType::LEFT_CURLY);
    while (true)
    {
        if (auto arg = parseExpression(scope))
        {
            callArgs.push_back(arg);
        }
        else if (!tryConsume(TokenType::COMMA))
        {
            break;
        }
    }


    consume(TokenType::RIGHT_CURLY);
    if (isSysCall)
    {
        return std::make_shared<SystemFunctionCallNode>(functionName, callArgs);
    }
    return std::make_shared<FunctionCallNode>(functionName, callArgs);
}

bool Parser::importUnit(std::string filename)
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
        for (auto &function: m_functionDefinitions)
        {

            if (function->functionSignature() == definition->functionSignature())
            {
                functionExists = true;
                break;
            }
        }
        if (!functionExists)
        {
            m_functionDefinitions.push_back(definition);
            m_known_function_names.push_back(definition->name());
        }
    }

    for (auto &[typeName, newType]: parser.m_typeDefinitions)
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

std::unique_ptr<UnitNode> Parser::parseUnit()
{
    try
    {
        bool isProgram = current().tokenType == TokenType::KEYWORD && iequals(current().lexical, "program");
        bool isUnit = current().tokenType == TokenType::KEYWORD && iequals(current().lexical, "unit");
        UnitType unitType = UnitType::UNIT;
        if (isProgram || isUnit)
        {
            if (isProgram)
                unitType = UnitType::PROGRAM;

            consume(TokenType::NAMEDTOKEN);
            auto unitName = std::string(current().lexical);

            consume(TokenType::SEMICOLON);

            if (unitName != "system")
            {
                importUnit("system.pas");
            }

            std::shared_ptr<BlockNode> blockNode = nullptr;
            while (hasNext())
            {
                const int scope = 0;
                if (tryConsumeKeyWord("type"))
                {
                    parseTypeDefinitions(scope);
                }
                else if (tryConsumeKeyWord("uses"))
                {
                    while (consume(TokenType::NAMEDTOKEN))
                    {
                        auto filename = std::string(current().lexical) + ".pas";
                        importUnit(filename);


                        if (!tryConsume(TokenType::COMMA))
                            break;
                    }
                    consume(TokenType::SEMICOLON);
                }
                else if (tryConsumeKeyWord("procedure"))
                {
                    m_functionDefinitions.emplace_back(parseFunctionDefinition(scope, false));
                }
                else if (tryConsumeKeyWord("function"))
                {
                    m_functionDefinitions.emplace_back(parseFunctionDefinition(scope, true));
                }
                else if (canConsumeKeyWord("const") || canConsumeKeyWord("var") || canConsumeKeyWord("begin"))
                {
                    blockNode = parseBlock(scope);
                    consume(TokenType::DOT);
                }
                else if (tryConsume(TokenType::T_EOF))
                {
                    break;
                }
                else
                {
                    m_errors.push_back(ParserError{
                            .file_name = m_file_path.string(),
                            .token = m_tokens[m_current + 1],
                            .message = "unexpected token found " +
                                       std::string(magic_enum::enum_name(m_tokens[m_current + 1].tokenType)) + "!"});
                    break;
                }
            }

            if (hasError())
            {
                throw ParserException(m_errors);
            }


            return std::make_unique<UnitNode>(unitType, unitName, m_functionDefinitions, m_typeDefinitions, blockNode);
        }

        m_errors.push_back(
                ParserError{.file_name = m_file_path.string(),
                            .token = m_tokens[m_current + 1],
                            .message = "unexpected expected token found " +
                                       std::string(magic_enum::enum_name(m_tokens[m_current + 1].tokenType)) + "!"});
        throw ParserException(m_errors);
    }
    catch (ParserException &e)
    {
        std::cerr << e.what();
    }
    return nullptr;
}
