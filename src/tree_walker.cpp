#include "tree_walker.h"
#include <iostream>

static void tree_walker_read_intro(TreeWalker& walker, tinyxml2::XMLElement* element)
{
    auto intro_element = element->FirstChildElement("intro");
    if (!intro_element) return;

    const char* intro_text = intro_element->GetText();
    walker.intro = intro_text ? intro_text : "";
}

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

static void tree_walker_read_results(TreeWalker& walker, tinyxml2::XMLElement* element)
{
    auto child = element->FirstChildElement("result");
    while (child)
    {
        const char* name = child->Attribute("name");
        const char* text = child->GetText();

        if (name && text)
            walker.results.emplace(name, text);

        // next
        child = child->NextSiblingElement("result");
    }
}

static tinyxml2::XMLElement* tree_walker_find_first_node(tinyxml2::XMLElement* element)
{
    NodeType type = parse_node_type(element->Name());
    if (type != NodeType::UNKNOWN) return element;

    auto child = element->FirstChildElement();
    while (child)
    {
        auto node = tree_walker_find_first_node(child);
        if (node) return node;

        // next
        child = child->NextSiblingElement();
    }
    return nullptr;
}

int tree_walker_load(TreeWalker& walker, const char* filename)
{
    tinyxml2::XMLDocument doc;
    auto result = doc.LoadFile("res/tree.xml");
    if (result != tinyxml2::XML_SUCCESS)
    {
        std::cout << "Failed to open file. (" << result << ")\n";
        return 0;
    }

    auto first_node = tree_walker_find_first_node(doc.RootElement());
    walker.root = parse_tree_node(first_node, NodeType::UNKNOWN);

    tree_walker_read_prompts(walker, first_node);
    tree_walker_read_results(walker, doc.RootElement());
    tree_walker_read_intro(walker, doc.RootElement());

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
            char* end;
            int val = strtol(answer.c_str(), &end, 0);
            if (end != &answer[0] + answer.size())
                std::cout << "Answer has to be a number.\n";
            else
                next = decision_tree_step(node, val);
        }

        if (!next)
            std::cout << "Invalid answer. Try Again.\n";
        else
            node = next;
    }

    return node ? node->name : "";
}

void tree_walker_show_intro(const TreeWalker& walker)
{
    std::cout << "DecisionTree:\n" << walker.intro << "\n";
}

void tree_walker_show_result(const TreeWalker& walker, const std::string& name)
{
    if (name.empty())
    {
        std::cout << "Something went wrong.\n";
        return;
    }

    auto result = walker.results.find(name);
    if (result != walker.results.end())
        std::cout << "Result:\n" << result->second << "\n";
    else
        std::cout << "Unkown result.\n";
}
