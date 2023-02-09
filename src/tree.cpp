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

int tree_node_eval(DecisionType type, TreeNodeValue value, int var)
{
    // check type
    auto expr = std::get_if<DecisionExpr>(&value);
    if (type != DecisionType::NUMERIC || !expr) return -1;

    // eval
    return decision_expr_eval(expr, var);
}

int tree_node_eval(DecisionType type, TreeNodeValue value, const std::string& var)
{
    // check type
    auto str_value = std::get_if<std::string>(&value);
    if (type != DecisionType::OPTION || !str_value) return -1;

    // eval
    return var.compare(*str_value) == 0;
}

const TreeNode* decision_tree_step(const TreeNode* node, int var)
{
    if (node->type == NodeType::FINAL) return node;
    if (node->decision != DecisionType::NUMERIC) return nullptr;

    for (const auto& choice : node->choices)
    {
        if (tree_node_eval(node->decision, choice.value, var))
            return &choice;
    }

    return nullptr;
}

const TreeNode* decision_tree_step(const TreeNode* node, const std::string& var)
{
    if (node->type == NodeType::FINAL) return node;
    if (node->decision != DecisionType::OPTION) return nullptr;

    for (const auto& choice : node->choices)
    {
        if (tree_node_eval(node->decision, choice.value, var))
            return &choice;
    }

    return nullptr;
}