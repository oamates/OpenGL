#ifndef MATRIX_3
#define MATRIX_3

#include <iostream>
#include "vector3.hpp"

struct Vector3;
struct Matrix3
{
        Matrix3();
        ~Matrix3();
        
        void setIdentity();
        void resetToZero();
        
        double det() const;
        static Matrix3 inverse(const Matrix3& parMatrix);
        Vector3 operator *= (const Vector3& parFactor);

        double ** m;
};
std::ostream& operator<< (std::ostream& os, const Matrix3& obj);

#endif //MATRIX_3
