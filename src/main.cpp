#include "tree.h"
#include "tree_walker.h"

const char* get_op_name(DecisionOp type)
{
    switch (type)
    {
    case DecisionOp::UNKNOWN: return "unknown";
    case DecisionOp::EQ:      return "=";
    case DecisionOp::NOTEQ:   return "!=";
    case DecisionOp::GT:      return ">";
    case DecisionOp::GTEQ:    return ">=";
    case DecisionOp::LT:      return "<";
    case DecisionOp::LTEQ:    return "<=";
    case DecisionOp::BETWEEN: return "between";
    }
    return "";
}

void print_node(const TreeNode& node, int level = 0)
{
    if (level > 0) printf("%*s-", level, " ");
    printf("Node: %s (%d)", node.name.c_str(), node.type);

    if (auto expr = std::get_if<DecisionExpr>(&node.value))
        printf(" - value: %s | %d;%d", get_op_name(expr->op), expr->value, expr->value2);
    else
        printf(" - value: %s", std::get<std::string>(node.value).c_str());

    putchar('\n');
    for (const auto& choice : node.choices)
        print_node(choice, level + 2);
}

void test_tree(const TreeWalker& walker, std::vector<const char*> answers, const char* expected)
{
    printf("Testing: ");
    for (auto answer : answers)
        printf("\"%s\" ", answer);

    putchar('\n');

    const TreeNode* node = &walker.root;
    for (auto answer : answers)
    {
        if (node->type == NodeType::FINAL)
        {
            printf("[Warn] Reached final node before asking all questions.\n");
            break;
        }

        if (node->type == NodeType::OPTION)
            node = decision_tree_step(node, answer);
        else if (node->type == NodeType::DECISION)
            node = decision_tree_step(node, std::stoi(answer));
    }

    if (!node)
    {
        if (expected) printf("[Error] Node is null.\n");
        else          printf("[Success] Reached null.\n");
        return;
    }

    if (node->type != NodeType::FINAL)
    {
        printf("[Warn] Reached non node after asking all questions.\n");
    }

    if (node->name.compare(expected) == 0)
        printf("[Success] Reached expected node (%s).\n", node->name.c_str());
    else
        printf("[Failed] Reached node: %s, expected: %s.\n", node->name.c_str(), expected);
}

void run_tests(const TreeWalker& walker)
{
    printf("===============================================\n");
    printf("| Decision Tree:                              |\n");
    printf("===============================================\n");
    print_node(walker.root);
    putchar('\n');
    printf("===============================================\n");
    printf("| Tests:                                      |\n");
    printf("===============================================\n");

    test_tree(walker, { "sunny", "10" }, "walk");
    test_tree(walker, { "sunny", "-10" }, nullptr);
    test_tree(walker, { "sunny", "200" }, "bus");
    test_tree(walker, { "cloudy", "yes" }, "walk");
    test_tree(walker, { "rain" }, "bus");
}

int main(int argc, char* argv[])
{
    const char* filename = argc > 1 ? argv[1] : "res/tree.xml";

    TreeWalker walker;
    if (!tree_walker_load(walker, filename))
        return -1;

    // run_tests(walker);

    tree_walker_show_intro(walker);
    auto result = tree_walker_run(walker);
    tree_walker_show_result(walker, result);

    return 0;
}