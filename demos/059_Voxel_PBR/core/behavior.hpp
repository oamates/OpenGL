#pragma once

#include "../core/engine_base.hpp"
#include "../types/instance_pool.hpp"

// Logic implemented in the Update method is meant to be implemented by all inheriting classes.
// UpdateAll will call all implementations of Update from instances of classes inheriting from Behavior
class Behavior : public InstancePool<Behavior>
{
    public:
        Behavior();
        virtual ~Behavior();
    protected:
        // Called per frame, contains the behavior's logic
        virtual void Update() = 0;
    private:
        friend void EngineBase::MainLoop() const;
        static void UpdateAll();
};

