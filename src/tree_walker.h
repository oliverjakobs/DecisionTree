#pragma once

#include "tree.h"

#include <map>

struct TreeWalker
{
    TreeNode root;
    std::string intro;
    std::map<std::string, std::string> prompts;
    std::map<std::string, std::string> results;
};

int tree_walker_load(TreeWalker& walker, const char* filename);

std::string tree_walker_run(const TreeWalker& walker);

void tree_walker_show_intro(const TreeWalker& walker);
void tree_walker_show_result(const TreeWalker& walker, const std::string& name);
