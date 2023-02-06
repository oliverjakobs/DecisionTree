#include "tinyxml2/tinyxml2.h"

#include <string>
#include <vector>
#include <variant>

using namespace tinyxml2;

enum class NodeType
{
    UNKNOWN,
    DECISION,
    FINAL
};

NodeType parse_node_type(const char* str)
{
    if (!str) return NodeType::UNKNOWN;

    if (strcmp("decision", str) == 0) return NodeType::DECISION;
    if (strcmp("final", str) == 0) return NodeType::FINAL;

    return NodeType::UNKNOWN;
}

enum class DecisionType
{
    ERROR,
    OPTION,
    NUMERIC,
    NONE
};

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
    EQ,
    NOTEQ,
    GT,
    GTEQ,
    LT,
    LTEQ,
    BETWEEN
};

const char* get_token_name(TokenType type)
{
    switch (type)
    {
    case TokenType::UNKNOWN: return "unknown";
    case TokenType::NUMBER:  return "number";
    case TokenType::GT:      return ">";
    case TokenType::GTEQ:    return ">=";
    case TokenType::LT:      return "<";
    case TokenType::LTEQ:    return "<=";
    case TokenType::EQ:      return "=";
    case TokenType::NOTEQ:   return "!=";
    case TokenType::BETWEEN: return "between";
    }
    return "";
}

struct Token
{
    TokenType type;
    const char* start;
    const char* end;
};

struct DecisionOp
{
    TokenType type;
    int value;
    int value2;
};

const char* next_token(Token& token, const char* cursor)
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
        TOKEN_CASE2('<', TokenType::LT, '=', TokenType::LTEQ)
        TOKEN_CASE2('>', TokenType::GT, '=', TokenType::GTEQ)
        TOKEN_CASE2('!', TokenType::UNKNOWN, '=', TokenType::NOTEQ)
        TOKEN_CASE2('.', TokenType::UNKNOWN, '.', TokenType::BETWEEN)
    }

    token.end = cursor;
    return cursor;

#undef TOKEN_CASE1
#undef TOKEN_CASE2
}

DecisionOp parse_decision_op(const char* str)
{
    Token token;
    const char* cursor = next_token(token, str);
    if (token.type == TokenType::UNKNOWN) return { TokenType::UNKNOWN, 0 };

    int value = 0;
    TokenType type = token.type;

    if (token.type == TokenType::NUMBER)
        value = std::stoi(std::string(token.start, token.end - token.start));

    cursor = next_token(token, cursor);
    if (token.type == TokenType::UNKNOWN) return { TokenType::UNKNOWN, 0 };

    if (token.type == TokenType::END_OF_LINE)
    {
        if (type == TokenType::NUMBER)  return { TokenType::EQ, value };
        else                            return { TokenType::UNKNOWN, 0 };
    }
    
    if (token.type == TokenType::BETWEEN)
    {
        cursor = next_token(token, cursor);
        if (token.type != TokenType::NUMBER) return { TokenType::UNKNOWN, 0 };

        int value2 = std::stoi(std::string(token.start, token.end - token.start));
        type = TokenType::BETWEEN;

        cursor = next_token(token, cursor);
        if (token.type == TokenType::END_OF_LINE)   return { TokenType::BETWEEN, value, value2 };
        else                                        return { TokenType::UNKNOWN, 0 };
    }

    if (token.type == TokenType::NUMBER)
    {
        value = std::stoi(std::string(token.start, token.end - token.start));

        cursor = next_token(token, cursor);
        if (token.type == TokenType::END_OF_LINE)   return { type, value };
        else                                        return { TokenType::UNKNOWN, 0 };
    }

    return { TokenType::UNKNOWN, 0 };
}

typedef std::variant<std::string, DecisionOp> DecisionValue;

DecisionValue parse_decision_value(const char* str)
{
    if (!str) return "";

    DecisionOp op = parse_decision_op(str);
    if (op.type != TokenType::UNKNOWN) return op;

    return std::string(str);
}

struct TreeNode
{
    NodeType type;
    std::string name;

    DecisionType decision;
    DecisionValue value;

    std::vector<TreeNode> choices;
};

TreeNode get_node_from_element(XMLElement* element, bool is_root = false)
{
    TreeNode node;
    node.type = parse_node_type(element->Name());
    node.name = element->Name();
    node.decision = DecisionType::ERROR;
    node.choices = {};

    if (node.type == NodeType::UNKNOWN) return node;

    if (node.type == NodeType::FINAL)
        node.name = element->GetText();
    else if (node.type == NodeType::DECISION)
        node.name = element->Attribute("name");

    node.decision = parse_decision_type(element->Attribute("type"));
    node.value = is_root ? "root" : parse_decision_value(element->Attribute("value"));

    return node;
}

void print_node(const TreeNode* node, int level = 0)
{
    if (level > 0) printf("%*s-", level, " ");
    printf("Node: %s (%d)", node->name.c_str(), node->decision);

    if (auto op = std::get_if<DecisionOp>(&node->value))
        printf(" - value: %s | %d;%d", get_token_name(op->type), op->value, op->value2);
    else
        printf(" - value: %s", std::get<std::string>(node->value).c_str());
    
    putchar('\n');
    for (const auto& choice : node->choices)
    {
        print_node(&choice, level+2);
    }
}

void parse_element(TreeNode* node, XMLElement* element)
{
    auto child = element->FirstChildElement();
    while (child)
    {
        auto choice = get_node_from_element(child);
        parse_element(&choice, child);
        node->choices.push_back(choice);
        child = child->NextSiblingElement();
    }
}

int main()
{
    XMLDocument doc;
    if (doc.LoadFile("res/tree.xml") != XML_SUCCESS)
    {
        printf("Failed to open file\n");
    }

    auto root_element = doc.RootElement();
    auto root = get_node_from_element(root_element, true);

    parse_element(&root, root_element);

    print_node(&root);

    return 0;
}