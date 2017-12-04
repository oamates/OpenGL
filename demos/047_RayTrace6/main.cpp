#include "log.hpp"
#include "renderer.hpp"

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		debug_color_msg(DEBUG_RED_COLOR, "Usage: %s [scene file path] [optional non-empty string]", argv[0]);
		return -1;
	}
	
	Renderer& theRenderer = Renderer::Instance();
	if(theRenderer.Init(argv[1], argv[2] != NULL))
		theRenderer.Run();
	return 0;
}
