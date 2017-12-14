#pragma once
#include <string>
#include <bitset>
#include <array>
#include <vector>

struct BaseObject
{
    BaseObject() {}
    virtual ~BaseObject() {}
    std::string name;                           // The object name
    std::array<std::bitset<32>, 64> mode;       // The object flags
};

