#include "tree_walker.h"
#include <iostream>

static void tree_walker_read_prompts(TreeWalker& walker, tinyxml2::XMLElement* element)
{
    const char* name = element->Attribute("name");
    auto prompt_element = element->FirstChildElement("prompt");
    const char* prompt = prompt_element ? prompt_element->GetText() : nullptr;

    if (name && prompt)
        walker.prompts.emplace(name, prompt);

    auto child = element->FirstChildElement();
    while (child)
    {
        NodeType type = parse_node_type(child->Name());

        if (type == NodeType::DECISION || type == NodeType::OPTION)
            tree_walker_read_prompts(walker, child);

        // next
        child = child->NextSiblingElement();
    }
}

static tinyxml2::XMLElement* tree_walker_find_root(tinyxml2::XMLElement* element)
{
    NodeType type = parse_node_type(element->Name());
    if (type != NodeType::UNKNOWN) return element;

    auto child = element->FirstChildElement();
    while (child)
    {
        auto root = tree_walker_find_root(child);
        if (root) return root;

        // next
        child = child->NextSiblingElement();
    }
    return nullptr;
}

int tree_walker_load(TreeWalker& walker, const char* filename)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile("res/tree.xml") != tinyxml2::XML_SUCCESS)
    {
        printf("Failed to open file\n");
        return 0;
    }

    auto root_element = tree_walker_find_root(doc.RootElement());
    walker.root = parse_tree_node(root_element, NodeType::UNKNOWN);
    tree_walker_read_prompts(walker, root_element);

    return 1;
}

std::string tree_walker_run(const TreeWalker& walker)
{
    const TreeNode* node = &walker.root;
    while (node)
    {
        if (node->type == NodeType::FINAL) break;

        auto prompt = walker.prompts.find(node->name);
        std::cout << (prompt != walker.prompts.end() ? prompt->second : node->name) << "\n";

        std::string answer = "";
        std::cin >> answer;

        const TreeNode* next = nullptr;
        if (node->type == NodeType::OPTION)
            next = decision_tree_step(node, answer);
        else if (node->type == NodeType::DECISION)
        {
            // TODO: safe conversion to int
            int var = std::stoi(answer);
            next = decision_tree_step(node, var);
        }

        if (!next)
            std::cout << "Invalid answer. Try Again.\n";
        else
            node = next;
    }

    return node ? node->name : "";
}