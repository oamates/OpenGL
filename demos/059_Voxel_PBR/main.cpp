#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <oglplus/error/program.hpp>

#include "core/engine_base.hpp"
#include "rendering/render_window.hpp"

int main(int argc, char* argv[])
{
  #ifndef NDEBUG    // DEBUG
    auto errorCaptured = false;                     // pause in positive case

    try                                             // instantiate engine core to load all assets and relevant data and start rendering main loop
    {
        EngineBase::Instance()->MainLoop();
    }
    catch (const oglplus::ProgramBuildError &pbe)
    {
        std::cerr << pbe.Log() << std::endl;
        errorCaptured = true;
    }
    catch (const oglplus::Error &err)
    {
        std::cerr << "Error (in " << err.GLFunc() << "') [" << err.SourceFile() << ":" << err.SourceLine() << "]: " << err.what() << std::endl;
        errorCaptured = true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        errorCaptured = true;
    }

    if (errorCaptured)
        std::cin.get();
  #else             // RELEASE
    EngineBase::Instance()->MainLoop();             // instantiate engine core to load all assets and relevant data and start rendering main loop
  #endif
    EngineBase::Terminate();    
    return 0;                                       // exit application
}

