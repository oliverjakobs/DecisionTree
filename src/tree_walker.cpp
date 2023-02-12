#include "tree_walker.h"
#include <iostream>

void TreeWalker::read_prompts(tinyxml2::XMLElement* root)
{
    const char* name = root->Attribute("name");
    auto prompt_element = root->FirstChildElement("prompt");
    const char* prompt = prompt_element ? prompt_element->GetText() : nullptr;

    if (name && prompt)
        prompts.emplace(name, prompt);

    auto child = root->FirstChildElement();
    while (child)
    {
        NodeType type = parse_node_type(child->Name());

        if (type == NodeType::DECISION || type == NodeType::OPTION)
        {
            read_prompts(child);
        }

        // next
        child = child->NextSiblingElement();
    }
}

TreeWalker::TreeWalker(const char* filename)
{
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile("res/tree.xml") != tinyxml2::XML_SUCCESS)
    {
        printf("Failed to open file\n");
        return;
    }

    root = parse_tree_node(doc.RootElement(), NodeType::UNKNOWN);
    read_prompts(doc.RootElement());
    
    
    for (const auto& p : prompts)
    {
        printf("%s: %s\n", p.first.c_str(), p.second.c_str());
    }
}

TreeWalker::~TreeWalker()
{

}

std::string TreeWalker::run() const
{
    const TreeNode* node = &root;
    while (node)
    {
        if (node->type == NodeType::FINAL) break;

        auto prompt = prompts.find(node->name);
        std::cout << (prompt != prompts.end() ? prompt->second : node->name) << "\n";

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
