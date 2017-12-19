
#include "core/engine.hpp"
#include "rendering/render_window.hpp"

int main(int argc, char* argv[])
{
    engine_t::instance()->mainloop();               // instantiate engine core to load all assets and relevant data and start rendering main loop
    engine_t::terminate();    
    return 0;                                       // exit application
}

