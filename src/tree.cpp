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
    if (node->type == NodeType::FINAL) return node;
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
    if (node->type == NodeType::FINAL) return node;
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