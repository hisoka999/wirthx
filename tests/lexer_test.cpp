#include "Lexer.h"
#include <gtest/gtest.h>
#include <magic_enum/magic_enum.hpp>
#include <string>
using namespace std::literals;


void PrintTo(const TokenType e, std::ostream *os) { *os << magic_enum::enum_name(e); }

TEST(LexerTest, LexHelloWorld)
{
    Lexer lexer;

    auto result = lexer.tokenize(R"(program Test;
    begin
        WriteLn('HelloWorld');
    end.)");

    EXPECT_EQ(result.size(), 15);
    ASSERT_EQ(result[0].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[0].lexical, "program"sv);
    ASSERT_EQ(result[1].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[1].lexical, "Test"sv);
}

TEST(LexerTest, LexNumbers)
{
    Lexer lexer;

    auto result = lexer.tokenize(R"(x := (1 + 2) * 5;)");

    EXPECT_EQ(result.size(), 12);
    ASSERT_EQ(result[0].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[0].lexical, "x"sv);
    ASSERT_EQ(result[1].tokenType, TokenType::COLON);
    ASSERT_EQ(result[1].lexical, ":"sv);
    ASSERT_EQ(result[2].tokenType, TokenType::EQUAL);
    ASSERT_EQ(result[2].lexical, "="sv);
    ASSERT_EQ(result[3].tokenType, TokenType::LEFT_CURLY);
    ASSERT_EQ(result[3].lexical, "("sv);
    ASSERT_EQ(result[4].tokenType, TokenType::NUMBER);
    ASSERT_EQ(result[4].lexical, "1"sv);
    ASSERT_EQ(result[5].tokenType, TokenType::PLUS);
    ASSERT_EQ(result[5].lexical, "+"sv);
    ASSERT_EQ(result[6].tokenType, TokenType::NUMBER);
    ASSERT_EQ(result[6].lexical, "2"sv);
    ASSERT_EQ(result[7].tokenType, TokenType::RIGHT_CURLY);
    ASSERT_EQ(result[7].lexical, ")"sv);
    ASSERT_EQ(result[8].tokenType, TokenType::MUL);
    ASSERT_EQ(result[8].lexical, "*"sv);
    ASSERT_EQ(result[9].tokenType, TokenType::NUMBER);
    ASSERT_EQ(result[9].lexical, "5"sv);
    ASSERT_EQ(result[10].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[10].lexical, ";"sv);
    ASSERT_EQ(result[11].tokenType, TokenType::T_EOF);
}

TEST(LexerTest, LexNegNumbers)
{
    Lexer lexer;

    auto result = lexer.tokenize(R"(x := (-1 + 2) * -5;)");

    EXPECT_EQ(result.size(), 12);
    ASSERT_EQ(result[0].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[0].lexical, "x"sv);
    ASSERT_EQ(result[1].tokenType, TokenType::COLON);
    ASSERT_EQ(result[1].lexical, ":"sv);
    ASSERT_EQ(result[2].tokenType, TokenType::EQUAL);
    ASSERT_EQ(result[2].lexical, "="sv);
    ASSERT_EQ(result[3].tokenType, TokenType::LEFT_CURLY);
    ASSERT_EQ(result[3].lexical, "("sv);
    ASSERT_EQ(result[4].tokenType, TokenType::NUMBER);
    ASSERT_EQ(result[4].lexical, "-1"sv);
    ASSERT_EQ(result[5].tokenType, TokenType::PLUS);
    ASSERT_EQ(result[5].lexical, "+"sv);
    ASSERT_EQ(result[6].tokenType, TokenType::NUMBER);
    ASSERT_EQ(result[6].lexical, "2"sv);
    ASSERT_EQ(result[7].tokenType, TokenType::RIGHT_CURLY);
    ASSERT_EQ(result[7].lexical, ")"sv);
    ASSERT_EQ(result[8].tokenType, TokenType::MUL);
    ASSERT_EQ(result[8].lexical, "*"sv);
    ASSERT_EQ(result[9].tokenType, TokenType::NUMBER);
    ASSERT_EQ(result[9].lexical, "-5"sv);
    ASSERT_EQ(result[10].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[10].lexical, ";"sv);
    ASSERT_EQ(result[11].tokenType, TokenType::T_EOF);
}

TEST(LexerTest, LexVarDeclaration)
{
    Lexer lexer;

    auto result = lexer.tokenize(R"(var x :integer;)");

    EXPECT_EQ(result.size(), 6);
    ASSERT_EQ(result[0].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[0].lexical, "var"sv);
    ASSERT_EQ(result[1].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[1].lexical, "x"sv);
    ASSERT_EQ(result[2].tokenType, TokenType::COLON);
    ASSERT_EQ(result[2].lexical, ":"sv);
    ASSERT_EQ(result[3].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[3].lexical, "integer"sv);
    ASSERT_EQ(result[4].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[4].lexical, ";"sv);
    ASSERT_EQ(result[5].tokenType, TokenType::T_EOF);
}

TEST(LexerTest, LexProcedureDeclaration)
{
    Lexer lexer;

    auto result = lexer.tokenize(R"(procedure MyProc;
    Begin
        {My Comment}
    END;)");

    EXPECT_EQ(result.size(), 10);
    ASSERT_EQ(result[0].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[0].lexical, "procedure"sv);
    ASSERT_EQ(result[0].row, 1);
    ASSERT_EQ(result[1].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[1].lexical, "MyProc"sv);
    ASSERT_EQ(result[2].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[2].lexical, ";"sv);
    ASSERT_EQ(result[3].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[4].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[4].lexical, "Begin"sv);
    ASSERT_EQ(result[4].row, 2);
    ASSERT_EQ(result[5].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[6].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[7].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[7].lexical, "END"sv);
    ASSERT_EQ(result[7].row, 4);
    ASSERT_EQ(result[8].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[8].lexical, ";"sv);

    ASSERT_EQ(result[9].tokenType, TokenType::T_EOF);
}

TEST(LexerTest, LexProcedureDeclarationWithArgs)
{
    Lexer lexer;

    auto result = lexer.tokenize(R"(procedure MyProc(arg1: integer; arg2 : real);
    Begin
        {My Comment}
    END;)");

    EXPECT_EQ(result.size(), 19);
    ASSERT_EQ(result[0].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[0].lexical, "procedure"sv);
    ASSERT_EQ(result[1].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[1].lexical, "MyProc"sv);
    ASSERT_EQ(result[2].tokenType, TokenType::LEFT_CURLY);
    ASSERT_EQ(result[2].lexical, "("sv);
    ASSERT_EQ(result[3].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[3].lexical, "arg1"sv);
    ASSERT_EQ(result[4].tokenType, TokenType::COLON);
    ASSERT_EQ(result[4].lexical, ":"sv);
    ASSERT_EQ(result[5].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[5].lexical, "integer"sv);
    ASSERT_EQ(result[6].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[6].lexical, ";"sv);

    ASSERT_EQ(result[7].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[7].lexical, "arg2"sv);
    ASSERT_EQ(result[8].tokenType, TokenType::COLON);
    ASSERT_EQ(result[8].lexical, ":"sv);
    ASSERT_EQ(result[9].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[9].lexical, "real"sv);
    ASSERT_EQ(result[10].tokenType, TokenType::RIGHT_CURLY);
    ASSERT_EQ(result[10].lexical, ")"sv);

    ASSERT_EQ(result[11].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[11].lexical, ";"sv);
    ASSERT_EQ(result[12].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[13].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[13].lexical, "Begin"sv);
    ASSERT_EQ(result[14].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[15].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[16].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[16].lexical, "END"sv);
    ASSERT_EQ(result[17].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[17].lexical, ";"sv);

    ASSERT_EQ(result[18].tokenType, TokenType::T_EOF);
}

TEST(LexerTest, LexFunctionDeclaration)
{
    Lexer lexer;

    auto result = lexer.tokenize(R"(function MyFunc : integer;
    Begin
        MyFunc := 100;
        {comment
        multiline}
    END;)");

    EXPECT_EQ(result.size(), 18);
    ASSERT_EQ(result[0].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[0].lexical, "function"sv);
    ASSERT_EQ(result[1].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[1].lexical, "MyFunc"sv);
    ASSERT_EQ(result[2].tokenType, TokenType::COLON);
    ASSERT_EQ(result[2].lexical, ":"sv);
    ASSERT_EQ(result[3].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[3].lexical, "integer"sv);

    ASSERT_EQ(result[4].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[4].lexical, ";"sv);
    ASSERT_EQ(result[5].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[6].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[6].lexical, "Begin"sv);
    ASSERT_EQ(result[7].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[8].tokenType, TokenType::NAMEDTOKEN);

    ASSERT_EQ(result[8].lexical, "MyFunc"sv);
    ASSERT_EQ(result[9].tokenType, TokenType::COLON);
    ASSERT_EQ(result[9].lexical, ":"sv);
    ASSERT_EQ(result[10].tokenType, TokenType::EQUAL);
    ASSERT_EQ(result[10].lexical, "="sv);
    ASSERT_EQ(result[11].tokenType, TokenType::NUMBER);
    ASSERT_EQ(result[11].lexical, "100"sv);
    ASSERT_EQ(result[12].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[12].lexical, ";"sv);
    ASSERT_EQ(result[13].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[14].tokenType, TokenType::ENDLINE);
    ASSERT_EQ(result[15].tokenType, TokenType::KEYWORD);
    ASSERT_EQ(result[15].lexical, "END"sv);
    ASSERT_EQ(result[15].row, 6);
    ASSERT_EQ(result[16].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[16].lexical, ";"sv);
    ASSERT_EQ(result[16].row, 6);

    ASSERT_EQ(result[17].tokenType, TokenType::T_EOF);
}
TEST(LexerTest, LexQuotedString)
{
    Lexer lexer;

    auto result = lexer.tokenize(R"(str4 := 'this is a ''quoted'' string'; )");

    EXPECT_EQ(result.size(), 6);
    ASSERT_EQ(result[0].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[1].tokenType, TokenType::COLON);
    ASSERT_EQ(result[2].tokenType, TokenType::EQUAL);
    ASSERT_EQ(result[3].tokenType, TokenType::STRING);
    ASSERT_EQ(result[3].lexical, "this is a ''quoted'' string"sv);
    ASSERT_EQ(result[4].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[5].tokenType, TokenType::T_EOF);
}


TEST(LexerTest, LexEscapedString)
{

    Lexer lexer;

    auto result = lexer.tokenize(R"(str4 := #13#10; )");

    EXPECT_EQ(result.size(), 6);
    ASSERT_EQ(result[0].tokenType, TokenType::NAMEDTOKEN);
    ASSERT_EQ(result[1].tokenType, TokenType::COLON);
    ASSERT_EQ(result[2].tokenType, TokenType::EQUAL);
    ASSERT_EQ(result[3].tokenType, TokenType::ESCAPED_STRING);
    ASSERT_EQ(result[3].lexical, "#13#10"sv);
    ASSERT_EQ(result[4].tokenType, TokenType::SEMICOLON);
    ASSERT_EQ(result[5].tokenType, TokenType::T_EOF);
}
