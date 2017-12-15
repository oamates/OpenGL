#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <oglplus/error/program.hpp>

#include "core/engine_base.hpp"
#include "rendering/render_window.hpp"

int main(int argc, char* argv[])
{
    EngineBase::Instance()->MainLoop();             // instantiate engine core to load all assets and relevant data and start rendering main loop
    EngineBase::Terminate();    
    return 0;                                       // exit application
}

