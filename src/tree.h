#pragma once

#include <string>
#include <vector>
#include <variant>

#include "tinyxml2/tinyxml2.h"

// ------------------------------------------------------------------------
// decision expr
// ------------------------------------------------------------------------
enum class DecisionOp
{
    UNKNOWN = 0,
    EQ = 10, NOTEQ,
    GT,      GTEQ,
    LT,      LTEQ,
    BETWEEN
};

struct DecisionExpr
{
    DecisionOp op;
    int value;
    int value2;
};

bool decision_expr_eval(const DecisionExpr* expr, int var);

// ------------------------------------------------------------------------
// tree
// ------------------------------------------------------------------------
enum class NodeType
{
    UNKNOWN,
    DECISION,
    OPTION,
    INVALID,
    FINAL
};

typedef std::variant<std::string, DecisionExpr> TreeNodeValue;

struct TreeNode
{
    NodeType type;
    std::string name;

    TreeNodeValue value;

    std::vector<TreeNode> choices;
};

const TreeNode* decision_tree_step(const TreeNode* node, int var);
const TreeNode* decision_tree_step(const TreeNode* node, const std::string& var);

// ------------------------------------------------------------------------
// parsing
// ------------------------------------------------------------------------
NodeType parse_node_type(const char* str);
TreeNode parse_tree_node(tinyxml2::XMLElement* element, NodeType parent_type);