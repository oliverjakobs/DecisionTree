
#include "tree.h"
#include "tinyxml2/tinyxml2.h"

#include <iostream>

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
    printf("Node: %s (%d)", node->name.c_str(), node->decision);

    if (auto expr = std::get_if<DecisionExpr>(&node->value))
        printf(" - value: %s | %d;%d", get_op_name(expr->op), expr->value, expr->value2);
    else
        printf(" - value: %s", std::get<std::string>(node->value).c_str());

    putchar('\n');
    for (const auto& choice : node->choices)
        print_node(&choice, level + 2);
}

TreeNode parse_tree_node(tinyxml2::XMLElement* element, bool is_root = false)
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
    node.value = is_root ? "root" : parse_value(element->Attribute("value"));

    auto child = element->FirstChildElement();
    while (child)
    {
        node.choices.push_back(parse_tree_node(child));
        child = child->NextSiblingElement();
    }

    return node;
}

int main()
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile("res/tree.xml") != tinyxml2::XML_SUCCESS)
    {
        printf("Failed to open file\n");
    }

    auto root = parse_tree_node(doc.RootElement(), true);

    print_node(&root);

    printf("\n\nDecicions:\n");

    const TreeNode* node = &root;
    while (node)
    {
        if (node->type == NodeType::FINAL) break;

        if (node->decision == DecisionType::OPTION)
        {
            std::cout << "Decision: " << node->name << " (option)\n";
            std::string answer = "";
            std::cin >> answer;

            node = decision_tree_step(node, answer);
        }
        else if (node->decision == DecisionType::NUMERIC)
        {
            std::cout << "Decision: " << node->name << " (numeric)\n";
            int answer = 0;
            std::cin >> answer;

            node = decision_tree_step(node, answer);
        }
    }
    printf("Decision done.\n");
    if (node)
        std::cout << "Result: " << node->name.c_str() << "\n";

    return 0;
}