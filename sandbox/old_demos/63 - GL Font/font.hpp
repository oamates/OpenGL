#ifndef _bmpfont_included_23589623423012623571290237235623823029235046702307354
#define _bmpfont_included_23589623423012623571290237235623823029235046702307354

struct bmpfont 
{
	bmpfont(const char *name);
	~bmpfont();
	
	void enable(int w, int h);
	void disable();
	
	void puts(float x, float y, const char *str);
	void printf(float x, float y, const char *format,...);
	void printfc(float x, float y, const char *format,...);
	
	GLuint tex_id;

	glsl_program
	
	int step;
	int width;
	int height;

	int space_left[256];
	int space_right[256];
	
	glm::mat4 modelview_matrix;
	glm::mat4 projection_matrix;
};

#endif // _bmpfont_included_23589623423012623571290237235623823029235046702307354 
