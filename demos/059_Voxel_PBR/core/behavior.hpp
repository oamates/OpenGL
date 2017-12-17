#pragma once

#include "../core/engine_base.hpp"
#include "../types/instance_pool.hpp"

// Logic implemented in the Update method is meant to be implemented by all inheriting classes.
// UpdateAll will call all implementations of Update from instances of classes inheriting from Behavior
struct Behavior : public InstancePool<Behavior>
{
    Behavior() {}
    virtual ~Behavior() {}

    virtual void Update() = 0;                              // Called per frame, contains the behavior's logic

    static void UpdateAll()
    {
        for (auto &behavior : instances)
            behavior->Update();
    }
};