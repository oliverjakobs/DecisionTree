#pragma once

#include "tree.h"


enum class TokenType
{
    END_OF_LINE = -1,
    UNKNOWN = 0,
    NUMBER,
    EQ,
    NOTEQ,
    GT,
    GTEQ,
    LT,
    LTEQ,
    BETWEEN
};

struct Token
{
    TokenType type;
    const char* start;
    const char* end;
};