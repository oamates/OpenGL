#pragma once

#include "../core/interface.hpp"

class UIFramerate : public Interface
{
    protected:
        void Draw() override;
    public:
        UIFramerate();
        ~UIFramerate();
};

