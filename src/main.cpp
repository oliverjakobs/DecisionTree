
#include "tree.h"
#include "tinyxml2/tinyxml2.h"

using namespace tinyxml2;

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