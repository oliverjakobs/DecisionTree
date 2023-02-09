#pragma once

#include <string>
#include <vector>
#include <variant>

// ------------------------------------------------------------------------
// decision
// ------------------------------------------------------------------------
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

enum class DecisionType
{
    ERROR,
    OPTION,
    NUMERIC,
    NONE
};

bool decision_expr_eval(const DecisionExpr* expr, int var);

// ------------------------------------------------------------------------
// tree
// ------------------------------------------------------------------------
enum class NodeType
{
    UNKNOWN,
    DECISION,
    FINAL
};

typedef std::variant<std::string, DecisionExpr> TreeNodeValue;

struct TreeNode
{
    NodeType type;
    std::string name;

    DecisionType decision;
    TreeNodeValue value;

    std::vector<TreeNode> choices;
};

int tree_node_eval(DecisionType type, TreeNodeValue value, int var);
int tree_node_eval(DecisionType type, TreeNodeValue value, const std::string& var);

typedef std::variant<std::string, int> DecisionVariable;

const TreeNode* decision_tree_step(const TreeNode* node, int var);
const TreeNode* decision_tree_step(const TreeNode* node, const std::string& var);

// ------------------------------------------------------------------------
// parser
// ------------------------------------------------------------------------
NodeType parse_node_type(const char* str);
DecisionType parse_decision_type(const char* str);
TreeNodeValue parse_value(const char* str);