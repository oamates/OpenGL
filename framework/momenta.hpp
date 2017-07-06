#ifndef _momenta_included_01563190253620562875649856438975643987568129635788781
#define _momenta_included_01563190253620562875649856438975643987568129635788781

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

//========================================================================================================================================================================================================================
// Jacobi algorithm of diagonalization of a symmetric 3x3 matrix A
// Computes quaternion q such that its corresponding 3x3 matrix Q diagonalizes A : D = Q * A * tr(Q), and A = tr(Q) * D * Q
// real_t should be either float or double
//========================================================================================================================================================================================================================

template<typename real_t> 
glm::dquat diagonalizer(const glm::tmat3x3<real_t>& A)
{
    glm::dquat q(1.0, 0.0, 0.0, 0.0);
    glm::dmat3 M = glm::dmat3(A);                                                       // cast the argument to perform all the calculations in double precision

    for(int i = 0; i < 16; i++)                                                         // this loop is executed ~7 times on average
    {
        glm::dmat3 Q = mat3_cast(q);
        glm::dmat3 D = Q * M * glm::transpose(Q);                                       // A = tr(Q) * D * Q

        double abs0 = glm::abs(D[1][2]), 
               abs1 = glm::abs(D[2][0]), 
               abs2 = glm::abs(D[0][1]);                                                // magnitudes of non-diagonal elements

        double theta;

        int k;

        if ((abs0 > abs1) && (abs0 > abs2))
        {
            k = 0;
            theta = 0.5 * (D[1][1] - D[2][2]) / D[1][2];
        }
        else if (abs1 > abs2)
        {
            k = 1;
            theta = 0.5 * (D[2][2] - D[0][0]) / D[2][0];
        }
        else
        {
            if (D[0][1] == 0) break;
            k = 2;
            theta = 0.5 * (D[0][0] - D[1][1]) / D[0][1];
        }

        glm::dquat jacobi(0.0, 0.0, 0.0, 0.0);                                          // jacobi rotation for this iteration

        if (theta > 0.0)
        {
            double t =  1.0 / ( theta + glm::sqrt(theta * theta + 1.0));                // sign(t) / (abs(t) + sqrt(t * t + 1))
            double c = glm::inversesqrt(t * t + 1.0);                                   // c = 1 / (t * t + 1), t = s / c 
            jacobi[k] = -glm::sqrt(0.5 - 0.5 * c);                                      // using identity sin(alpha / 2) = sqrt((1 - cos(a)) / 2)
        }
        else
        {
            double t = -1.0 / (-theta + glm::sqrt(theta * theta + 1.0)) ;               // sign(t) / (abs(t) + sqrt(t * t + 1))
            double c = glm::inversesqrt(t * t + 1.0);                                   // c = 1 / (t * t + 1), t = s / c 
            jacobi[k] =  glm::sqrt(0.5 - 0.5 * c);                                      // using identity sin(alpha / 2) = sqrt((1 - cos(a)) / 2)
        }

        jacobi.w = glm::sqrt(1.0 - jacobi[k] * jacobi[k]);
        if (jacobi.w == 1.0) break;                                                     // ... reached machine precision
        q = jacobi * q;  
        q = glm::normalize(q);
    } 
    return q;
}

namespace momenta
{

//========================================================================================================================================================================================================================
// Calculates 1st and second order momenta of a point cloud
//========================================================================================================================================================================================================================

template<typename vertex_t> 
void calculate(const vertex_t* vertices, int N, glm::dvec3& mass_center, glm::dmat3& covariance_matrix)
{
    mass_center = glm::dvec3(0.0);
    double sxx = 0.0, sxy = 0.0, sxz = 0.0,
           syy = 0.0, syz = 0.0, szz = 0.0;

    for (int i = 0; i < N; ++i)
    {
        glm::dvec3 position = glm::dvec3(vertices[i].position);
        mass_center += position;
        
        sxx += position.x * position.x;
        sxy += position.x * position.y;
        sxz += position.x * position.z;
        syy += position.y * position.y;
        syz += position.y * position.z;
        szz += position.z * position.z;
    }

    double inv_n = 1.0 / N;
    
    mass_center *= inv_n;

    double mxx = inv_n * sxx - mass_center.x * mass_center.x;
    double mxy = inv_n * sxy - mass_center.x * mass_center.y;
    double mxz = inv_n * sxz - mass_center.x * mass_center.z;
    double myy = inv_n * syy - mass_center.y * mass_center.y;
    double myz = inv_n * syz - mass_center.y * mass_center.z;
    double mzz = inv_n * szz - mass_center.z * mass_center.z;
    
    covariance_matrix = glm::dmat3(mxx, mxy, mxz,
                                   mxy, myy, myz,
                                   mxz, myz, mzz);

}

template<> 
void calculate<glm::dvec3>(const glm::dvec3* vertices, int N, glm::dvec3& mass_center, glm::dmat3& covariance_matrix)
{
    mass_center = glm::dvec3(0.0);
    double sxx = 0.0, sxy = 0.0, sxz = 0.0,
           syy = 0.0, syz = 0.0, szz = 0.0;

    for (int i = 0; i < N; ++i)
    {
        glm::dvec3 position = vertices[i];
        mass_center += position;
        
        sxx += position.x * position.x;
        sxy += position.x * position.y;
        sxz += position.x * position.z;
        syy += position.y * position.y;
        syz += position.y * position.z;
        szz += position.z * position.z;
    }

    double inv_n = 1.0 / N;
    
    mass_center *= inv_n;

    double mxx = inv_n * sxx - mass_center.x * mass_center.x;
    double mxy = inv_n * sxy - mass_center.x * mass_center.y;
    double mxz = inv_n * sxz - mass_center.x * mass_center.z;
    double myy = inv_n * syy - mass_center.y * mass_center.y;
    double myz = inv_n * syz - mass_center.y * mass_center.z;
    double mzz = inv_n * szz - mass_center.z * mass_center.z;
    
    covariance_matrix = glm::dmat3(mxx, mxy, mxz,
                                   mxy, myy, myz,
                                   mxz, myz, mzz);

}

template<typename vertex_t> 
void calculate(const std::vector<vertex_t>& points, glm::dvec3& mass_center, glm::dmat3& covariance_matrix)
{
    calculate(points.data(), static_cast<int>(points.size()), mass_center, covariance_matrix);
}

//========================================================================================================================================================================================================================
// Function that normalizes the point cloud to have center of mass at origin and principal momenta axes  
//========================================================================================================================================================================================================================

template<typename real_t>
void axis_align(glm::tvec3<real_t>* points, int N)
{
    glm::dvec3 mass_center;
    glm::dmat3 covariance_matrix;

    calculate(points, N, mass_center, covariance_matrix);
    
    glm::dquat q = diagonalizer(covariance_matrix);
    glm::dmat3 Q = mat3_cast(q);
    
    for (int i = 0; i < N; ++i)
    {
        glm::dvec3& point = points[i];        
        points[i] = glm::tvec3<real_t>(Q * (point - mass_center));
    }
}

template<typename real_t> 
void axis_align(std::vector<glm::tvec3<real_t>>& points)
{
    axis_align(points.data(), static_cast<int>(points.size()));
}

} // namespace momenta

#endif // _momenta_included_01563190253620562875649856438975643987568129635788781
