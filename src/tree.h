#pragma once

#include <string>
#include <vector>
#include <variant>

// ------------------------------------------------------------------------
// decision
// ------------------------------------------------------------------------
enum class DecisionType
{
    ERROR,
    OPTION,
    NUMERIC,
    NONE
};

enum class DecisionOp
{
    UNKNOWN = 0,
    EQ = 10,
    NOTEQ,
    GT,
    GTEQ,
    LT,
    LTEQ,
    BETWEEN
};

struct DecisionExpr
{
    DecisionOp op;
    int value;
    int value2;
};

typedef std::variant<std::string, DecisionExpr> DecisionValue;

struct Decision
{
    DecisionType type;
    DecisionValue value;
};


// ------------------------------------------------------------------------
// tree
// ------------------------------------------------------------------------
enum class NodeType
{
    UNKNOWN,
    DECISION,
    FINAL
};

struct TreeNode
{
    NodeType type;
    std::string name;

    DecisionType decision;
    DecisionValue value;

    std::vector<TreeNode> choices;
};


// ------------------------------------------------------------------------
// parser
// ------------------------------------------------------------------------
NodeType parse_node_type(const char* str);
DecisionType parse_decision_type(const char* str);
DecisionValue parse_decision_value(const char* str);