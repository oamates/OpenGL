#pragma once

#include "../rendering/interface_renderer.hpp"
#include "../core/engine_base.hpp"
#include "../types/instance_pool.hpp"

/// <summary>
/// Interface code in the <see cref="Draw"/>
/// method is meant to be implemented by all inheriting classes.
/// <see cref="DrawAll"/> will call all implementations of <see cref="Draw"/>
/// from instances of classes inheriting from <see cref="Interface"/>
/// </summary>
/// <seealso cref="InterfaceRenderer" />
/// <seealso cref="InstancePool{Interface}" />
class Interface : InterfaceRenderer, public InstancePool<Interface>
{
    public:
        Interface();
        virtual ~Interface();
    protected:
        ImGuiIO * io;
        ImGuiStyle * style;
        /// <summary>
        /// Called per frame, interface drawing logic resides on this method
        /// </summary>
        virtual void Draw() = 0;
    private:
        friend void EngineBase::MainLoop() const;
        static void DrawAll();
};
