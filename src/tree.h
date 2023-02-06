#pragma once

enum class NodeType
{
    UNKNOWN,
    DECISION,
    FINAL
};

enum class DecisionType
{
    ERROR,
    OPTION,
    NUMERIC,
    NONE
};