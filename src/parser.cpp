#include "tree.h"
#include "tinyxml2/tinyxml2.h"

// ------------------------------------------------------------------------
// basic parsing
// ------------------------------------------------------------------------
static NodeType parse_node_type(const char* str)
{
    if (!str) return NodeType::UNKNOWN;

    if (strcmp("decision", str) == 0) return NodeType::DECISION;
    if (strcmp("option", str) == 0) return NodeType::OPTION;
    if (strcmp("final", str) == 0) return NodeType::FINAL;

    return NodeType::UNKNOWN;
}

// ------------------------------------------------------------------------
// expression parsing
// ------------------------------------------------------------------------
enum class TokenType
{
    END = -1,
    UNKNOWN = 0,
    NUMBER,
    EQ = 10, NOTEQ,
    GT,      GTEQ,
    LT,      LTEQ,
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
        token.type = TokenType::END;
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

int token_to_int(const Token* token)
{
    return std::stoi(std::string(token->start, token->end - token->start));
}

static DecisionExpr parse_decision_expr(const char* str)
{
    if (!str) return { DecisionOp::UNKNOWN, 0 };

    Token token;
    const char* cursor = next_token(token, str);
    if (token.type == TokenType::UNKNOWN) return { DecisionOp::UNKNOWN, 0 };

    int value = 0;
    TokenType type = token.type;

    if (token.type == TokenType::NUMBER)
        value = token_to_int(&token);

    cursor = next_token(token, cursor);
    if (token.type == TokenType::UNKNOWN) return { DecisionOp::UNKNOWN, 0 };

    // parse EQUAL (EQ) shortcut:
    //  - first and only token has to be NUMBER
    if (token.type == TokenType::END)
    {
        if (type == TokenType::NUMBER)  return { DecisionOp::EQ, value };
        else                            return { DecisionOp::UNKNOWN, 0 };
    }

    // parse BETWEEN expression: 
    //  - first token has to be NUMBER token
    //  - second token has to be BETWEEN token
    //  - third (and last) token has to be NUMBER token
    if (token.type == TokenType::BETWEEN)
    {
        cursor = next_token(token, cursor);
        if (token.type != TokenType::NUMBER) return { DecisionOp::UNKNOWN, 0 };

        int value2 = token_to_int(&token);
        type = TokenType::BETWEEN;

        cursor = next_token(token, cursor);
        if (token.type == TokenType::END)   return { DecisionOp::BETWEEN, value, value2 };
        else                                return { DecisionOp::UNKNOWN, 0 };
    }

    // parse basic expressions:
    //  - first token define operation
    //  - second (and last) token has to be NUMBER token
    if (token.type == TokenType::NUMBER)
    {
        value = token_to_int(&token);

        cursor = next_token(token, cursor);
        if (token.type == TokenType::END)   return { to_decision_op(type), value };
        else                                return { DecisionOp::UNKNOWN, 0 };
    }

    return { DecisionOp::UNKNOWN, 0 };
}

// ------------------------------------------------------------------------
// xml parsing
// ------------------------------------------------------------------------
static TreeNode parse_tree_node(tinyxml2::XMLElement* element, NodeType parent_type)
{
    TreeNode node;
    node.type = parse_node_type(element->Name());
    node.name = element->Attribute("name") ? element->Attribute("name") : element->Name();
    node.choices = {};

    if (node.type == NodeType::UNKNOWN) return node;

    if (node.type == NodeType::FINAL)
        node.name = element->GetText();

    if (parent_type == NodeType::DECISION)
    {
        auto expr = parse_decision_expr(element->Attribute("value"));
        if (expr.op == DecisionOp::UNKNOWN)
        {
            printf("[warn] Dropped Node %s (%d): ", node.name.c_str(), node.type);
            printf("Unkown operation.\n");
            return { NodeType::UNKNOWN };
        }
        node.value = expr;
    }
    else if (parent_type == NodeType::OPTION)
    {
        auto str = element->Attribute("value");
        if (!str)
        {
            printf("[warn] Dropped Node %s (%d): ", node.name.c_str(), node.type);
            printf("Missing value.\n");
            return { NodeType::UNKNOWN };
        }
        node.value = str;
    }

    // parse choices
    tinyxml2::XMLElement* child = element->FirstChildElement();
    while (child)
    {
        TreeNode choice = parse_tree_node(child, node.type);
        if (choice.type != NodeType::UNKNOWN)
            node.choices.push_back(choice);
        child = child->NextSiblingElement();
    }

    if (node.choices.empty())
        node.type = NodeType::FINAL;

    return node;
}

TreeNode parse_decision_tree(const char* filename)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile("res/tree.xml") != tinyxml2::XML_SUCCESS)
    {
        printf("Failed to open file\n");
        return { NodeType::UNKNOWN, "" };
    }

    return parse_tree_node(doc.RootElement(), NodeType::UNKNOWN);
}