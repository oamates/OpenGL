#ifndef _ICAMERARENDER_
#define _ICAMERARENDER_

#include "Classes.h"

template <class Matrix, class Vector, class Value> struct ICameraRender
{
	virtual ~ICameraRender() {};
	virtual void updateCamera(CameraModel<Matrix, Vector, Value>* camera) = 0;
};



#endif