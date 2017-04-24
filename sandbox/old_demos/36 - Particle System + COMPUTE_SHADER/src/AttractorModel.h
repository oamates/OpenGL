#ifndef _ATTRACTORMODEL_
#define _ATTRACTORMODEL_

template <class Vector, class Value> struct AttractorModel
{
	Vector	_position;
	float 	_attDepth; //The z-Coordinate of the attractor

	AttractorModel() = default;

	AttractorModel(const Vector& position) : _position(position) {};

	Vector	getPosition() const
		{ return _position; };

	//Override me
	virtual void setPosition(const Vector& position)
		{ _position = position; };

	void incrementDepth(const float inc)
	{
		_attDepth += inc;

		if(_attDepth < 0.0f){
			_attDepth = 0.0f;
		} else if (_attDepth > 1000.0f){
			_attDepth = 1000.0f;
		}
	}

	float getDepth() const
		{ return _attDepth; };
};

#endif