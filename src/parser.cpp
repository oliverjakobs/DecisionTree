#include "tree.h"


NodeType parse_node_type(const char* str)
{
    if (!str) return NodeType::UNKNOWN;

    if (strcmp("decision", str) == 0) return NodeType::DECISION;
    if (strcmp("final", str) == 0) return NodeType::FINAL;

    return NodeType::UNKNOWN;
}


DecisionType parse_decision_type(const char* str)
{
    if (!str) return DecisionType::NONE;

    if (strcmp("option", str) == 0) return DecisionType::OPTION;
    if (strcmp("numeric", str) == 0) return DecisionType::NUMERIC;

    return DecisionType::ERROR;
}

enum class TokenType
{
    END_OF_LINE = -1,
    UNKNOWN = 0,
    NUMBER,
    EQ = 10,
    NOTEQ,
    GT,
    GTEQ,
    LT,
    LTEQ,
    BETWEEN,
};

struct Token
{
    TokenType type;
    const char* start;
    const char* end;
};

static const char* next_token(Token& token, const char* cursor)
{
#define TOKEN_CASE1(c, t) case c: token.type = t; break;
#define TOKEN_CASE2(c1, t1, c2, t2) case c1:          \
    token.type = t1;                                  \
    if (*cursor == c2) { token.type = t2; cursor++; } \
    break;

    token.type = TokenType::UNKNOWN;
    token.start = cursor;
    token.end = nullptr;

    if (*cursor == '\0')
    {
        token.type = TokenType::END_OF_LINE;
        return cursor;
    }

    switch (*(cursor++))
    {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
        while (*cursor >= '0' && *cursor <= '9') cursor++;
        token.type = TokenType::NUMBER;
        break;
    }
    TOKEN_CASE1('=', TokenType::EQ)
        TOKEN_CASE1(':', TokenType::BETWEEN)
        TOKEN_CASE2('<', TokenType::LT, '=', TokenType::LTEQ)
        TOKEN_CASE2('>', TokenType::GT, '=', TokenType::GTEQ)
        TOKEN_CASE2('!', TokenType::UNKNOWN, '=', TokenType::NOTEQ)
    }

    token.end = cursor;
    return cursor;

#undef TOKEN_CASE1
#undef TOKEN_CASE2
}

static DecisionOp to_decision_op(TokenType type)
{
    if (type >= TokenType::EQ && type <= TokenType::BETWEEN)
        return static_cast<DecisionOp>(type);
    return DecisionOp::UNKNOWN;
}

static DecisionExpr parse_decision_expr(const char* str)
{
    Token token;
    const char* cursor = next_token(token, str);
    if (token.type == TokenType::UNKNOWN) return { DecisionOp::UNKNOWN, 0 };

    int value = 0;
    TokenType type = token.type;

    if (token.type == TokenType::NUMBER)
        value = std::stoi(std::string(token.start, token.end - token.start));

    cursor = next_token(token, cursor);
    if (token.type == TokenType::UNKNOWN) return { DecisionOp::UNKNOWN, 0 };

    if (token.type == TokenType::END_OF_LINE)
    {
        if (type == TokenType::NUMBER)  return { DecisionOp::EQ, value };
        else                            return { DecisionOp::UNKNOWN, 0 };
    }

    if (token.type == TokenType::BETWEEN)
    {
        cursor = next_token(token, cursor);
        if (token.type != TokenType::NUMBER) return { DecisionOp::UNKNOWN, 0 };

        int value2 = std::stoi(std::string(token.start, token.end - token.start));
        type = TokenType::BETWEEN;

        cursor = next_token(token, cursor);
        if (token.type == TokenType::END_OF_LINE)   return { DecisionOp::BETWEEN, value, value2 };
        else                                        return { DecisionOp::UNKNOWN, 0 };
    }

    if (token.type == TokenType::NUMBER)
    {
        value = std::stoi(std::string(token.start, token.end - token.start));

        cursor = next_token(token, cursor);
        if (token.type == TokenType::END_OF_LINE)   return { to_decision_op(type), value };
        else                                        return { DecisionOp::UNKNOWN, 0 };
    }

    return { DecisionOp::UNKNOWN, 0 };
}

TreeNodeValue parse_value(const char* str)
{
    if (!str) return "";

    DecisionExpr expr = parse_decision_expr(str);
    if (expr.op != DecisionOp::UNKNOWN) return expr;

    return std::string(str);
}