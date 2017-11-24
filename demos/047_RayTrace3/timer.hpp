#pragma once

#include <GL/glew.h> 

class OpenGLTimer
{
	GLuint startQuery, endQuery;
	GLuint64 startTime, endTime;
	GLuint64 elapsedTime;

  public:
	OpenGLTimer();
	~OpenGLTimer();

	void Start();
	void End();
	GLuint64 GetElapsedTime();
};

