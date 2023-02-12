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

void print_node(const TreeNode* node, int level = 0)
{
    if (level > 0) printf("%*s-", level, " ");
    printf("Node: %s (%d)", node->name.c_str(), node->type);

    if (auto expr = std::get_if<DecisionExpr>(&node->value))
        printf(" - value: %s | %d;%d", get_op_name(expr->op), expr->value, expr->value2);
    else
        printf(" - value: %s", std::get<std::string>(node->value).c_str());

    putchar('\n');
    for (const auto& choice : node->choices)
        print_node(&choice, level + 2);
}

void test_tree(const TreeWalker* walker, std::vector<const char*> answers, std::string expected)
{
    printf("Testing: ");
    for (auto answer : answers)
        printf("\"%s\" ", answer);

    putchar('\n');

    const TreeNode* node = walker->get_root();
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
        printf("[Error] Node is null.\n");
        return;
    }

    if (node->type != NodeType::FINAL)
    {
        printf("[Warn] Reached non node after asking all questions.\n");
    }

    if (expected.compare(node->name) == 0)
        printf("[Success] Reached expected node (%s).\n", node->name.c_str());
    else
        printf("[Failed] Reached node: %s, expected: %s.\n", node->name.c_str(), expected.c_str());

}

void run_tests(const TreeWalker* walker)
{
    printf("\n\nTests:\n");

    test_tree(walker, { "sunny", "10" }, "bus");
    test_tree(walker, { "sunny", "-10" }, "stay");
    test_tree(walker, { "sunny", "200" }, "walk");
    test_tree(walker, { "cloudy", "yes" }, "walk");
    test_tree(walker, { "rain" }, "bus");
}

int main()
{
    TreeWalker walker{ "res/tree.xml" };

    print_node(walker.get_root());
    //run_tests(&walker);

    auto result = walker.run();

    if (!result.empty())
        printf("Result: %s\n", result.c_str());
    else
        printf("Something went wrong.\n");

    return 0;
}