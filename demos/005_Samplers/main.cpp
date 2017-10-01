//========================================================================================================================================================================================================================
// DEMO 005 : Sampler objects and texture filtering
//========================================================================================================================================================================================================================

#include <cstdlib>
#include <cstdio>

#include "shader_mgmt.hpp"

//#define CAPTURE_VIDEO

int filtering_mode = 2;
int render_scene = 1;
int split_screen = 1;
int num_probes = 6;
float num_texels = 1.0;
float glob_speed = 0.001;
float filter_width = 1.0;
float filter_sharpness = 2.0;
int vsync = 1;
int showHelp = 0;
int showOSD = 1;
int supports_gl4 = 0;
int max_anisotropy = 16;
int uniform_frame = 0;
int uniform_time = 0;
float fps = 60;
float global_time = 0;

struct Texture
{
	GLubyte	*imageData;										
	GLuint	bpp;										
	GLuint	width;										
	GLuint	height;										
	GLuint	texID;
	
	void Bind()
        { glBindTexture(GL_TEXTURE_2D, texID); }

	bool LoadTGA(char* filename, GLenum MINF = GL_LINEAR, GLenum MAGF = GL_LINEAR, int mipmap = 1)
    {

	    GLubyte		TGAheader[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
	    GLubyte		TGAcompare[12];								
	    GLubyte		header[6];									
	    GLuint		bytesPerPixel;								
	    GLuint		imageSize;									
	    GLuint		temp;										
	    GLuint		type=GL_RGBA;

	    FILE* file = fopen(filename, "rb");
	
	    if(!file)
		    return 0;

	    if(fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||	
		   memcmp(TGAheader,TGAcompare,sizeof(TGAheader)) != 0				    ||	
		   fread(header, 1, sizeof(header), file) != sizeof(header))			
	    {
		    fclose(file);
		    return 0;
	    }

	    width  = header[1] * 256 + header[0];
	    height = header[3] * 256 + header[2];
    
 	    if(width <= 0 || height	<= 0 ||	(header[4] != 24 && header[4] != 32))                   // Is The TGA 24 or 32 Bit?
	    {
		    fclose(file);										
		    return 0;									
	    }

	    bpp	= header[4];							
	    bytesPerPixel = bpp / 8;					
	    imageSize = width * height * bytesPerPixel;	

	    imageData = (GLubyte*) malloc(imageSize);		                                        // Reserve Memory To Hold The TGA Data

	    if(imageData == NULL || fread(imageData, 1, imageSize, file) != imageSize)	
	    {
		    if(imageData != NULL)					
			    free(imageData);						

		    fclose(file);									
		    return 0;									
	    }

	    for(GLuint i = 0; i < int(imageSize); i += bytesPerPixel)	    	                    // Loop Through The Image Data
	    {														                                // Swap The 1st And 3rd Bytes ('R'ed and 'B'lue)
		    temp=imageData[i];							                                        // Temporarily Store The Value At Image Data 'i'
		    imageData[i] = imageData[i + 2];	                                                // Set The 1st Byte To The Value Of The 3rd Byte
		    imageData[i + 2] = temp;					                                        // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	    }

	    fclose (file);											                                // Close The File

	    glGenTextures(1, &texID);				

	    if (bpp == 24)								
		    type = GL_RGB;										

	    glBindTexture(GL_TEXTURE_2D, texID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MINF);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MAGF);

		glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE,imageData);


	    return true;											
    }

};	



struct demo_window_t : public imgui_window_t
{
    demo_window_t(const char* title, int glfw_samples, int version_major, int version_minor, int res_x, int res_y, bool fullscreen = true)
        : imgui_window_t(title, glfw_samples, version_major, version_minor, res_x, res_y, fullscreen, true)
    {
        gl_info::dump(OPENGL_BASIC_INFO | OPENGL_EXTENSIONS_INFO);        
    }

    //===================================================================================================================================================================================================================
    // event handlers
    //===================================================================================================================================================================================================================
    void on_key(int key, int scancode, int action, int mods) override
    {
	switch (key) {
	case 27:
		exit(0);
		break;
	case '1':
		printf("hardware texture filtering\n");
		filtering_mode = 1;
		break;
	case '2':
		printf("high quality elliptical texture filtering\n");
		filtering_mode = 2;
		break;
	case '3':
		printf("high quality elliptical texture filtering (linear)\n");
		filtering_mode = 3;
		break;
	case '4':
		printf("high quality elliptical texture filtering (bilinear)\n");
		filtering_mode = 4;
		break;
	case '5':
		printf("approximated elliptical texture filtering\n");
		filtering_mode = 5;
		break;
	case '6':
		printf("spatial elliptical texture filtering\n");
		filtering_mode = 6;
		break;
	case '7':
		printf("temporal elliptical texture filtering\n");
		filtering_mode = 7;
		break;
	case '8':
		printf("absolute lod error display\n");
		filtering_mode = 8;
		num_texels = 1;
		max_anisotropy = 16; //just to match NVIDIA hardware
		break;
	case '9':
		printf("anisotropy level\n");
		filtering_mode = 9;
		break;
	case '0':
		printf("MIP-MAP level\n");
		filtering_mode = 0;
		break;
	case 'a':
	case 'A':
		toggle(render_scene);
		if(render_scene==1){
			filter_width=1.0;
			filter_sharpness=2.0f;
			num_probes=6;
			num_texels = 1;
			max_anisotropy = 16;
		}
		else{
			filter_width=1.8f;
			filter_sharpness=5.5;
			num_probes=10;
			num_texels = 1;
			max_anisotropy = 16;
		}
		break;
	case 's':
	case 'S':
		printf("toggle split screen\n");
		toggle(split_screen);
		break;
	case '=':
	case '+':
		glob_speed+=0.001f;
		printf("speed: %f\n", glob_speed);
		break;
	case '-':
		glob_speed-=0.001f;
		printf("speed: %f\n", glob_speed);
		break;
	case 'e':
		num_probes-=2;
		printf("num_probes: %d\n", num_probes);
		break;
	case 'E':
		num_probes+=2;
		printf("num_probes: %d\n", num_probes);
		break;
	case 'q':
		filter_width-=0.1f;
		printf("speed: %f\n", filter_width);
		break;
	case 'Q':
		filter_width+=0.1f;
		printf("filter_width: %f\n", filter_width);
		break;
	case 'r':
		num_texels-=0.1f;
		break;
	case 'R':
		num_texels+=0.1f;
		break;
	case 't':
		max_anisotropy-=1;
		break;
	case 'T':
		max_anisotropy+=1;
		break;
	case 'h':
	case 'H':
		toggle(showHelp);
		break;
	case 'd':
	case 'D':
		toggle(showOSD);
		break;
	case 'w':
		filter_sharpness-=0.1f;
		break;
	case 'W':
		filter_sharpness+=0.1f;
		break;
	case 'v':
	case 'V':
		printf("toggle vertical sync\n");
		toggle(vsync);
		if(vsync==0){
			wglSwapIntervalEXT(0);
		}
		else{
			wglSwapIntervalEXT(1);
		}
		break;
	}

    }

    /*
    void on_mouse_move() override
    {
    }
    */

    void update_ui() override
    {
        //===============================================================================================================================================================================================================
        // show a simple fps window.
        //===============================================================================================================================================================================================================
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


	    if(showOSD)
        {
	        overlay->Begin();

		    DisplayFilterType();

			
			if(frameCounter==timingNumFrames){
				int timeInterval=glutGet(GLUT_ELAPSED_TIME)-timeCounter;
				fps = float(timingNumFrames)*1000.0f/float(timeInterval);
				timeCounter=glutGet(GLUT_ELAPSED_TIME);
				frameCounter=0;
				
			}	

			static char tmpString[128];
			sprintf(tmpString,"fps: %.1f",fps);
			overlay->Print(RESX-130,10,1,tmpString);

		if(filtering_mode<9)
			DisplayFilterInfo();

		if(showHelp)
			DisplayHelp();
	overlay->End();
	}


    }

};


void toggle(int& foo)
{
	if(foo)
		foo=0;
	else
		foo=1;
}

//Full screen quad vertices definition
const GLfloat quadVArray[] =
{
	-1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f,
	-1.0f,  1.0f, 0.0f, 1.0f,    
	 1.0f, -1.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 1.0f,
	-1.0f,  1.0f, 0.0f, 1.0f  
};


GLuint tunnelTexID=0;

//Shaders
GLuint tunnelProg=0;

//Timing
const int timingNumFrames=60;	//Use 0 to disable
uint frameCounter=0;
int timeCounter;

//Mesh VBOs
GLuint vertexBufferName=0;

char* fb_mem;
char file_name[256];

//write a ppm file to disk
void writeppm(char* file_name, int resx, int resy, char* raster)
{

	FILE * out = fopen(file_name, "wb");
	fprintf(out, "P6 %d %d 255\n", resy, resx);

	for(int i=0; i<resy; i++)
		for(int j=0; j<resx; j++)
		{
			putc(raster[3 * (i * resx + j) + 0], out);
			putc(raster[3 * (i * resx + j) + 1], out);
			putc(raster[3 * (i * resx + j) + 2], out);
		}

	fclose(out);
}

// save frame buffer to disk (used to capture video)
void saveFrameBuffer()
{
	glFinish();
	glReadBuffer( GL_BACK );
	glReadPixels( 0, 0, RESX, RESY, GL_RGB, GL_UNSIGNED_BYTE, fb_mem);
	sprintf(file_name,"capture%05d.ppm", frameCounter);
	writeppm(file_name, RESX, RESY, fb_mem);
}

void initShaders(void)
{
	// Shader dynamic macro setting
	resetShadersGlobalMacros();
	setShadersGlobalMacro("RENDER_SCENE", render_scene);
	setShadersGlobalMacro("FILTERING_MODE", filtering_mode);
	setShadersGlobalMacro("SPLIT_SCREEN", split_screen);
	setShadersGlobalMacro("USE_GL4", supports_gl4);
	setShadersGlobalMacro("SPEED", glob_speed);
	setShadersGlobalMacro("FILTER_WIDTH", filter_width);
	setShadersGlobalMacro("NUM_PROBES", num_probes);
	setShadersGlobalMacro("FILTER_SHARPNESS", filter_sharpness);
	setShadersGlobalMacro("TEXELS_PER_PIXEL", num_texels);
	setShadersGlobalMacro("MAX_ECCENTRICITY", max_anisotropy);

	// Shaders loading
	tunnelProg = createShaderProgram("Shaders/pass.vert", "Shaders/ewa.frag", tunnelProg );
	uniform_frame = glGetUniformLocation(tunnelProg, "frame");
	uniform_time = glGetUniformLocation(tunnelProg, "time");

}

unsigned char*	raster;
unsigned short	width;
unsigned short	height;
unsigned short	bpp;

int loadTexture(char* textureName)
{
	unsigned char	TGAheader[12] = {0,0,2,0,0,0,0,0,0,0,0,0};
	unsigned char	TGAcompare[12];								
	unsigned char	header[6];									
	unsigned short	bytesPerPixel;								
	unsigned int	imageSize;			
	GLuint			type = GL_RGBA;

	FILE* file = fopen(textureName, "rb");
	
	if(!file)
		return 0;

	if(	fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	
		fread(header,1,sizeof(header),file)!=sizeof(header))			
	{
		fclose(file);
		printf("ERROR loading texture file %s\n", textureName);
		return 0;
	}

	width  = header[1] * 256 + header[0];
	height = header[3] * 256 + header[2];

	if(	width	<=0	||								
		height	<=0	||								
		(header[4]!=24 && header[4]!=32))					// Is The TGA 24 or 32 Bit?
	{
		fclose(file);		
		printf("ERROR loading texture file %s\n", textureName);
		return 0;									
	}

	bpp	= header[4];							
	bytesPerPixel	= bpp/8;					
	imageSize		= width*height*bytesPerPixel;	

	raster = new unsigned char[imageSize];		// Reserve Memory To Hold The TGA Data

	if(	raster==NULL ||							
		fread(raster, 1, imageSize, file)!=imageSize)	
	{
		if(raster!=NULL)					
			delete [] raster;						

		fclose(file);		
		printf("ERROR loading texture file %s\n", textureName);
		return 0;									
	}
	return 1;

}

void drawQuad(GLuint prog) {

	glUseProgram(tunnelProg);
	glUniform1i(uniform_frame,frameCounter);
	glUniform1f(uniform_time,global_time);

	glEnableVertexAttribArray (glGetAttribLocation(prog, "vertexPos"));

	glBindBuffer (GL_ARRAY_BUFFER, vertexBufferName);

	glVertexAttribPointer (glGetAttribLocation(prog, "vertexPos"), 4, GL_FLOAT, GL_FALSE,
		sizeof(GLfloat)*4, 0);

	glDrawArrays(GL_TRIANGLES, 0, 24);

}


void DisplayFilterInfo(){
	char tmpString[128];
	if(filtering_mode<8){

		sprintf(tmpString,"Filter Width     : %.1f",filter_width);
		overlay->Print(10,10,1,tmpString);
		sprintf(tmpString,"Filter Sharpness : %.1f",filter_sharpness);
		overlay->Print(10,35,1,tmpString);
		if(filtering_mode==2 || filtering_mode==3 || filtering_mode==4 )
			sprintf(tmpString,"Number of Probes : adaptive");
		else if (filtering_mode==1)
			sprintf(tmpString,"Number of Probes : %d",1);
		else if (filtering_mode==7)
			sprintf(tmpString,"Number of Probes : %d",3);	
		else 
			sprintf(tmpString,"Number of Probes : %d",num_probes-1);
		overlay->Print(10,60,1,tmpString);
		sprintf(tmpString,"Texels/Pixel     : %.1f", num_texels);
		overlay->Print(10,85,1,tmpString);
		sprintf(tmpString,"Max Anisotropy   : %d", max_anisotropy);
		overlay->Print(10,110,1,tmpString);
		overlay->Print(10,145,1,"(press h for help)");

	}
	else{
		sprintf(tmpString,"Texels/Pixel: %.1f", num_texels);
		overlay->Print(10,10,1,tmpString);
		sprintf(tmpString,"Max Anisotropy: %d", max_anisotropy);
		overlay->Print(10,35,1,tmpString);
	}
}

void DisplayHelp(){
	overlay->Print(100,200,1,"[1-7] Change filtering mode");
	overlay->Print(100,230,1,"[ 8 ] Hardware mip-map selection deviation");
	overlay->Print(100,260,1,"[ 9 ] Visualize anisotropy level");
	overlay->Print(100,290,1,"[q/Q] Increase / Decrease filter width");
	overlay->Print(100,320,1,"[w/W] Increase / Decrease filter sharpness");
	overlay->Print(100,350,1,"[e/E] Increase / Decrease number of probes");
	overlay->Print(100,380,1,"[r/R] Increase / Decrease texels per pixel");
	overlay->Print(100,410,1,"[t/T] Increase / Decrease maximum anisotropy");
	overlay->Print(100,440,1,"[-/+] Increase / Decrease camera speed");
	overlay->Print(100,470,1,"[ h ] Toggle help");
	overlay->Print(100,500,1,"[ s ] Toggle split screen");
	overlay->Print(100,530,1,"[ d ] Toggle On Screen Display (OSD)");
	overlay->Print(100,560,1,"[ v ] Toggle vsync");
	overlay->Print(100,590,1,"[ a ] Toggle between infinite tunnel or planes");
}

void DisplayFilterType(){
	char tmpString[128];

	switch (filtering_mode){
	case 1:
		sprintf(tmpString,"Hardware Filtering");
		break;
	case 2:
		sprintf(tmpString,"Elliptical Filtering");
		break;
	case 3:
		sprintf(tmpString,"Elliptical Filtering (2-tex)");
		break;
	case 4:
		sprintf(tmpString,"Elliptical Filtering (4-tex)");
		break;
	case 5:
		sprintf(tmpString,"Approximated Elliptical Filter");
		break;
	case 6:
		sprintf(tmpString,"Spatial Elliptical Filtering");
		break;
	case 7:
		sprintf(tmpString,"Temporal Elliptical Filtering");
		break;
	case 8:
		if(supports_gl4)
			sprintf(tmpString,"Mip-map selection absolute deviation x2 (black = zero error)");
		else
			sprintf(tmpString,"Error: Requires an OpenGL 4 context");
		break;
	case 9:
		sprintf(tmpString,"Anisotropy level (pure red > 16)");
		break;
	case 0:
		sprintf(tmpString,"Mip-Map level visualization");
		break;
	};
	overlay->Print(10,RESY-35,1,tmpString);
	if(split_screen && filtering_mode<8)
		overlay->Print(RESX-220,RESY-35,1,"Hardware Filtering");
}


//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    const int res_x = 1920;
    const int res_y = 1080;


    //===================================================================================================================================================================================================================
    // initialize GLFW library, create GLFW window and initialize GLEW library
    // 4AA samples, OpenGL 3.3 context, screen resolution : 1920 x 1080, fullscreen
    //===================================================================================================================================================================================================================
    if (!glfw::init())
        exit_msg("Failed to initialize GLFW library. Exiting ...");

    demo_window_t window("GLFW + OpenGL 3.3 + ImGui Example", 1, 3, 3, res_x, res_y);

    //===================================================================================================================================================================================================================
    // Shader and uniform variables initialization
    //===================================================================================================================================================================================================================
    glsl_program_t ray_marcher(glsl_shader_t(GL_VERTEX_SHADER,   "glsl/ray_marcher.vs"),
                               glsl_shader_t(GL_FRAGMENT_SHADER, "glsl/canyon.fs"));

	//Full screen quad initialization
	glGenBuffers (1, &vertexBufferName);
	glBindBuffer (GL_ARRAY_BUFFER, vertexBufferName);
	glBufferData (GL_ARRAY_BUFFER, sizeof(quadVArray), quadVArray, GL_STATIC_DRAW);

	//load the texture
	if(!loadTexture("res/checker.tga"))
		exit(1);
	if(!tunnelTexID)
		glGenTextures(1, &tunnelTexID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tunnelTexID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, raster);
	glGenerateMipmap(GL_TEXTURE_2D);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

    //===================================================================================================================================================================================================================
    // main program loop
    //===================================================================================================================================================================================================================
    while(!window.should_close())
    {
        //===============================================================================================================================================================================================================
        // clear back buffer, process events and update timer
        //===============================================================================================================================================================================================================
    	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	    glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        window.new_frame();

        //===============================================================================================================================================================================================================
	    // render the full screen quad
        //===============================================================================================================================================================================================================
	    glBindTexture(GL_TEXTURE_2D, tunnelTexID);
	    drawQuad(tunnelProg);

	    glUseProgram (0);
	    frameCounter++;
	    global_time = window.frame_ts;

      #ifdef CAPTURE_VIDEO
	    saveFrameBuffer();
      #endif
        window.end_frame();
    }

    //===================================================================================================================================================================================================================
    // terminate the program and exit
    //===================================================================================================================================================================================================================
    glfw::terminate();
    return 0;
}