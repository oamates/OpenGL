#pragma once

#include "../core/interface.hpp"

class UIFramebuffers : public Interface
{
    protected:
        void Draw() override;
    public:
        UIFramebuffers();
        ~UIFramebuffers();
};

