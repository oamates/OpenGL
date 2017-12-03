#ifndef VECTOR3
#define VECTOR3

#include <iostream>
#include "matrix3.hpp"

struct Matrix3;
struct Vector3
{
		Vector3();
		Vector3(double parX, double parY, double parZ);
		Vector3(const Vector3& parVec);
		~Vector3();
		
		Vector3& operator*=(double parFactor);
		Vector3& operator/=(double parFactor);
		Vector3 operator*(double parFactor) const;
		Vector3 operator/(double parFactor) const;
		Vector3 operator+(const Vector3& parVect) const;
        Vector3 operator-(const Vector3& parVect) const;
		Vector3 operator*(const Matrix3& parMatrix);
		static double dotProduct(const Vector3& parV1, const Vector3& parV2);
		static Vector3 crossProduct(const Vector3& parV1, const Vector3& parV2);
		
		double Norm();
		double x;
		double y;
		double z;
	
};
std::ostream& operator << (std::ostream& os, const Vector3& obj); 
#endif
