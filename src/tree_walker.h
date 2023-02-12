#pragma once

#include "tree.h"

#include <map>

class TreeWalker
{
private:
    TreeNode root;
    std::map<std::string, std::string> prompts;

    void read_prompts(tinyxml2::XMLElement* root);
public:
    TreeWalker(const char* filename);
    ~TreeWalker();

    std::string run() const;

    const TreeNode* get_root() const { return &root; };
};