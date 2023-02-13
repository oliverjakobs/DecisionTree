#pragma once

#include "tree.h"

#include <map>

struct TreeWalker
{
    TreeNode root;
    std::map<std::string, std::string> prompts;
};

int tree_walker_load(TreeWalker& walker, const char* filename);

std::string tree_walker_run(const TreeWalker& walker);
