//========================================================================================================================================================================================================================
// DEMO 058 : SDF Texture 3D raymarcher
//========================================================================================================================================================================================================================
#define GLM_FORCE_RADIANS 
#define GLM_FORCE_NO_CTOR_INIT

#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/transform.hpp>

#include "log.hpp"

template<typename real_t> real_t volume(const glm::tvec3<real_t>& A, const glm::tvec3<real_t>& B, const glm::tvec3<real_t>& C)
    { return glm::determinant(glm::tmat3x3<real_t>(A, B, C)); }

//=======================================================================================================================================================================================================================
// unsigned distance-to-triangle function
//=======================================================================================================================================================================================================================
template<typename real_t> real_t triangle_udf(const glm::tvec3<real_t>& P, const glm::tvec3<real_t>& A, const glm::tvec3<real_t>& B, const glm::tvec3<real_t>& C)
{
    const real_t zero = 0.0;
    const real_t  one = 1.0;

    glm::tvec3<real_t> AB = B - A; glm::tvec3<real_t> AP = P - A;
    glm::tvec3<real_t> BC = C - B; glm::tvec3<real_t> BP = P - B;
    glm::tvec3<real_t> CA = A - C; glm::tvec3<real_t> CP = P - C;

    glm::tvec3<real_t> n = glm::cross(CA, AB);

    if ((volume(n, AB, AP) >= zero) && (volume(n, BC, BP) >= zero) && (volume(n, CA, CP) >= zero))
        return glm::sqrt(glm::dot(n, AP) * glm::dot(n, AP) / glm::length2(n));

    return glm::sqrt(
        glm::min(
            glm::min(
                glm::length2(AB * glm::clamp(glm::dot(AB, AP) / glm::length2(AB), zero, one) - AP),
                glm::length2(BC * glm::clamp(glm::dot(BC, BP) / glm::length2(BC), zero, one) - BP)
            ), 
            glm::length2(CA * glm::clamp(glm::dot(CA, CP) / glm::length2(CA), zero, one) - CP)
        )
    );
}

//=======================================================================================================================================================================================================================
// extended unsigned distance-to-triangle function
// returns both the vector from closest point in ABC to P (in xyz components) and the distance (in w component)
//=======================================================================================================================================================================================================================
template<typename real_t> glm::tvec4<real_t> tri_closest_point(const glm::tvec3<real_t>& P, const glm::tvec3<real_t>& A, const glm::tvec3<real_t>& B, const glm::tvec3<real_t>& C)
{
    const real_t zero = 0.0;
    const real_t  one = 1.0;

    glm::tvec3<real_t> AB = B - A; glm::tvec3<real_t> AP = P - A;
    glm::tvec3<real_t> BC = C - B; glm::tvec3<real_t> BP = P - B;
    glm::tvec3<real_t> CA = A - C; glm::tvec3<real_t> CP = P - C;

    glm::tvec3<real_t> n = glm::cross(CA, AB);

    if ((volume(n, AB, AP) >= zero) && (volume(n, BC, BP) >= zero) && (volume(n, CA, CP) >= zero))
    {
        n = normalize(n);
        real_t dp = glm::dot(n, AP);
        return glm::tvec4<real_t> (dp * n, glm::abs(dp));
    }

    glm::tvec3<real_t> proj_AB = AP - AB * glm::clamp(glm::dot(AB, AP) / glm::length2(AB), zero, one);
    glm::tvec3<real_t> proj_BC = BP - BC * glm::clamp(glm::dot(BC, BP) / glm::length2(BC), zero, one);
    glm::tvec3<real_t> proj_CA = CP - CA * glm::clamp(glm::dot(CA, CP) / glm::length2(CA), zero, one);

    real_t dAB = glm::length(proj_AB);
    real_t dBC = glm::length(proj_BC);
    real_t dCA = glm::length(proj_CA);

    if (dAB > dBC)
    {
        if (dCA > dBC) return glm::tvec4<real_t>(proj_BC, dBC);
    }
    else
    {
        if (dCA > dAB) return glm::tvec4<real_t>(proj_AB, dAB);
    }
    return glm::tvec4<real_t>(proj_CA, dCA);
}

std::mt19937 generator(452387519);
std::normal_distribution<> gauss_rand;

double gaussrand1d()
{
    return gauss_rand(generator);
}

glm::dvec3 gaussrand3d()
{
    return glm::dvec3(gauss_rand(generator), gauss_rand(generator), gauss_rand(generator));
}

//=======================================================================================================================================================================================================================
// program entry point
//=======================================================================================================================================================================================================================
int main(int argc, char *argv[])
{
    const double eps = 0.000000001;
    const double scale = 2.25;

    for(int i = 0; i < 1024 * 1024; ++i)
    {
        glm::dvec3 A = gaussrand3d();
        glm::dvec3 B = gaussrand3d();
        glm::dvec3 C = gaussrand3d();

        glm::dvec3 P = scale * gaussrand3d();


        double d_abc = triangle_udf(P, A, B, C);
        double d_cab = triangle_udf(P, C, A, B);
        double d_bca = triangle_udf(P, B, C, A);

        //===============================================================================================================================================================================================================
        // Test 0 : length should be greater than projection onto normal
        //===============================================================================================================================================================================================================
        if ((glm::abs(d_abc - d_cab) > eps) || 
            (glm::abs(d_cab - d_bca) > eps) || 
            (glm::abs(d_bca - d_abc) > eps))
        {
            debug_msg("Test 0 failed :: d_abc = %f, d_cab = %f, d_bca = %f", d_abc, d_cab, d_bca);
        }

        //===============================================================================================================================================================================================================
        // Test 1 : length should be greater than projection onto normal
        //===============================================================================================================================================================================================================
        glm::dvec3 n = glm::normalize(glm::cross(B - A, C - A));
        double proj = glm::abs(glm::dot(P - A, n));

        if ((d_abc + eps < proj) || (d_cab + eps < proj) || (d_bca + eps < proj))
        {
            debug_msg("Test 1 failed :: d_abc = %f, d_cab = %f, d_bca = %f, proj = %f", d_abc, d_cab, d_bca, proj);
        }

        //===============================================================================================================================================================================================================
        // Test 2 : length should be less than distances to vertices
        //===============================================================================================================================================================================================================
        double lAP = glm::length(P - A);
        double lBP = glm::length(P - B);
        double lCP = glm::length(P - C);

        if ((d_abc > lAP + eps) || (d_cab > lAP + eps) || (d_bca > lAP + eps) || 
            (d_abc > lBP + eps) || (d_cab > lBP + eps) || (d_bca > lBP + eps) || 
            (d_abc > lCP + eps) || (d_cab > lCP + eps) || (d_bca > lCP + eps))
        {
            debug_msg("Test 2 failed :: d_abc = %f, d_cab = %f, d_bca = %f\n\t\t\t\tlAP = %f, lBP = %f, lCP = %f", d_abc, d_cab, d_bca, lAP, lBP, lCP);
        }

        //===============================================================================================================================================================================================================
        // Test 3 : length should be less than distances to vertices
        //===============================================================================================================================================================================================================

        for(int i = 0; i < 8; ++i)
        {
            glm::dvec3 uvw = glm::abs(gaussrand3d());
            double q = 1.0 / (uvw.x + uvw.y + uvw.z);
            uvw *= q;

            glm::dvec3 Q = uvw.x * A + uvw.y * B + uvw.z * C;
            double lQP = glm::length(Q - P);

            if (lQP + eps < d_abc)
            {
                debug_msg("Test 3 failed :: d_abc = %f, d_cab = %f, d_bca = %f\n\t\t\t\tlQP = %f", d_abc, d_cab, d_bca, lQP);

            }

        }

        //===============================================================================================================================================================================================================
        // Test 4 : length should be less than distances to vertices
        //===============================================================================================================================================================================================================

        glm::dvec4 cp = tri_closest_point(P, A, B, C);
        double d2 = cp.w;
        glm::dvec3 grad = glm::dvec3(cp);
        double d3 = glm::length(grad);

        if ((glm::abs(d_abc - d2) > eps) || (glm::abs(d_bca - d2) > eps) || (glm::abs(d_cab - d2) > eps))
        {
            debug_msg("Test 4 failed :: d_abc = %f, d_cab = %f, d_bca = %f, d2 = %f", d_abc, d_cab, d_bca, d2);
        }

        if (glm::abs(d3 - d2) > eps)
        {
            debug_msg("Test 4 failed :: d3 = %f, d2 = %f", d3, d2);
        }

        //===============================================================================================================================================================================================================
        // Test 5 : length should grow if P shifted along distance gradient
        //===============================================================================================================================================================================================================

        glm::dvec3 P1 = P + 0.125 * glm::normalize(grad);
        double d5 = triangle_udf(P1, A, B, C); 
        
        if ((d5 <= d_abc) || (d5 >= d_abc + 0.125 + eps))
        {
            debug_msg("Test 5 failed :: d5 = %f, d = %f", d5, d_abc);
        }      
    }
}