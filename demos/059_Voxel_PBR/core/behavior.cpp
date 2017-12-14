#include "behavior.hpp"

Behavior::Behavior()
{
}

Behavior::~Behavior()
{
}

void Behavior::UpdateAll()
{
    for (auto &behavior : instances)
    {
        behavior->Update();
    }
}
