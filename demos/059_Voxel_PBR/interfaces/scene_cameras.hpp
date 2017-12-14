#pragma once

#include "../core/interface.hpp"

class UISceneCameras : public Interface
{
    protected:
        void Draw() override;
    public:
        UISceneCameras();
        ~UISceneCameras();
};

