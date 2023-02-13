#include "tree.h"

bool decision_expr_eval(const DecisionExpr* expr, int var)
{
    switch (expr->op)
    {
    case DecisionOp::EQ:    return var == expr->value;
    case DecisionOp::NOTEQ: return var != expr->value;
    case DecisionOp::GT:    return var > expr->value;
    case DecisionOp::GTEQ:  return var >= expr->value;
    case DecisionOp::LT:    return var < expr->value;
    case DecisionOp::LTEQ:  return var <= expr->value;
    case DecisionOp::BETWEEN:
        return expr->value <= var && var <= expr->value2;
    }
    return false;
}

const TreeNode* decision_tree_step(const TreeNode* node, int var)
{
    if (node->type != NodeType::DECISION) return nullptr;

    for (const auto& choice : node->choices)
    {
        auto expr = std::get_if<DecisionExpr>(&choice.value);
        if (!expr) return nullptr;

        if (decision_expr_eval(expr, var))
            return &choice;
    }

    return nullptr;
}

const TreeNode* decision_tree_step(const TreeNode* node, const std::string& var)
{
    if (node->type != NodeType::OPTION) return nullptr;

    for (const auto& choice : node->choices)
    {
        auto str_val = std::get_if<std::string>(&choice.value);
        if (!str_val) return nullptr;

        if (var.compare(*str_val) == 0)
            return &choice;
    }

    return nullptr;
}


// ------------------------------------------------------------------------
// basic parsing
// ------------------------------------------------------------------------
NodeType parse_node_type(const char* str)
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
    GT, GTEQ,
    LT, LTEQ,
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

static int token_to_int(const Token& t)
{
    return std::stoi(std::string(t.start, t.end - t.start));
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
        value = token_to_int(token);

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

        int value2 = token_to_int(token);
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
        value = token_to_int(token);

        cursor = next_token(token, cursor);
        if (token.type == TokenType::END)   return { to_decision_op(type), value };
        else                                return { DecisionOp::UNKNOWN, 0 };
    }

    return { DecisionOp::UNKNOWN, 0 };
}

// ------------------------------------------------------------------------
// xml parsing
// ------------------------------------------------------------------------
static std::vector<TreeNode> parse_choices(tinyxml2::XMLElement* child, NodeType parent_type)
{
    auto choices = std::vector<TreeNode>();
    while (child)
    {
        TreeNode choice = parse_tree_node(child, parent_type);

        // ignore unknown nodes
        if (choice.type != NodeType::UNKNOWN)
            choices.push_back(choice);

        // next
        child = child->NextSiblingElement();
    }
    return choices;
}

TreeNode parse_tree_node(tinyxml2::XMLElement* element, NodeType parent_type)
{
    // parse type
    NodeType type = parse_node_type(element->Name());
    if (type == NodeType::UNKNOWN) return { NodeType::UNKNOWN };

    // parse name
    const char* name = element->Attribute("name");

    // parse value
    TreeNodeValue value;
    if (parent_type == NodeType::DECISION)
    {
        // parent_type DECISION only allows expr as values
        auto expr = parse_decision_expr(element->Attribute("value"));
        if (expr.op == DecisionOp::UNKNOWN)
        {
            printf("[warn] Dropped node %s (%d): ", name, type);
            printf("Unkown operation.\n");
            return { NodeType::UNKNOWN };
        }
        value = expr;
    }
    else if (parent_type == NodeType::OPTION)
    {
        // parent_type OPTION only allows strings as values
        auto str = element->Attribute("value");
        if (!str)
        {
            printf("[warn] Dropped node %s (%d): ", name, type);
            printf("Missing value.\n");
            return { NodeType::UNKNOWN };
        }
        value = str;
    }

    // parse choices
    auto choices = parse_choices(element->FirstChildElement(), type);
    if (!choices.empty() && type == NodeType::FINAL)
    {
        printf("[warn] Found final node (%s) with children.\n", name);
        choices.clear();
    }
    // nodes without choices are final
    else if (choices.empty())
        type = NodeType::FINAL;

    // create node and return it
    TreeNode node;
    node.type = type;
    node.name = name ? name : "";
    node.value = value;
    node.choices = choices;
    return node;
}