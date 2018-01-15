#include "spherical_harmonics.hpp"

#include <glm/gtx/norm.hpp>

#include <iostream>
#include <limits>
#include <random>

#include "constants.hpp"

struct matrix
{
    double* data;
    matrix(int size_x, int size_y) {}

    int rows() const { return 0; }
    double& operator () (int i, int j) { return data[0]; }
    const double& operator () (int i, int j) const { return data[0]; }
};


namespace sh {

namespace {

//========================================================================================================================================================================================================================
// Number of precomputed factorials and double-factorials that can be returned in constant time.
//========================================================================================================================================================================================================================
const int kCacheSize = 16;
const int kHardCodedOrderLimit = 4;
const int kIrradianceOrder = 2;
const int kIrradianceCoeffCount = GetCoefficientCount(kIrradianceOrder);

//========================================================================================================================================================================================================================
// For efficiency, the cosine lobe for normal = (0, 0, 1) as the first 9
// spherical harmonic coefficients are hardcoded below. This was computed by
// evaluating:
//   ProjectFunction(kIrradianceOrder, [] (double phi, double theta) {
//     return Clamp(Eigen::Vector3d::UnitZ().dot(ToVector(phi, theta)),
//                  0.0, 1.0);
//   }, 10000000);
//========================================================================================================================================================================================================================
const std::vector<double> cosine_lobe = { 0.886227, 0.0, 1.02333, 0.0, 0.0, 0.0,
                                          0.495416, 0.0, 0.0 };

//========================================================================================================================================================================================================================
// A zero template is required for EvalSHSum to handle its template instantiations and a type's default constructor does not necessarily initialize to zero.
//========================================================================================================================================================================================================================
template<typename T> T Zero();

template<> double Zero()
    { return 0.0; }
template<> float Zero()
    { return 0.0f; }
template<> glm::vec3 Zero()
    { return glm::vec3(0.0f); }
template<> glm::dvec3 Zero()
    { return glm::dvec3(0.0); }

//template <class T>
//using VectorX = Eigen::Matrix<T, Eigen::Dynamic, 1>;

//========================================================================================================================================================================================================================
// Usage: CHECK(bool, string message);
// Note that it must end a semi-colon, making it look like a
// valid C++ statement (hence the awkward do() while(false)).
//========================================================================================================================================================================================================================
/*
#ifndef NDEBUG
# define CHECK(condition, message) \
  do { \
    if (!(condition)) { \
      std::cerr << "Check failed (" #condition ") in " << __FILE__ \
        << ":" << __LINE__ << ", message: " << message << std::endl; \
      std::exit(EXIT_FAILURE); \
    } \
  } while(false)
#else
# define ASSERT(condition, message) do {} while(false)
#endif
*/
//========================================================================================================================================================================================================================
// Clamp the first argument to be greater than or equal to the second and less than or equal to the third.
//========================================================================================================================================================================================================================
double Clamp(double val, double min, double max)
{
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

//========================================================================================================================================================================================================================
// Return true if the first value is within epsilon of the second value.
//========================================================================================================================================================================================================================
bool NearByMargin(double actual, double expected)
{
    double diff = actual - expected;
    if (diff < 0.0) {
        diff = -diff;
    }
    // 5 bits of error in mantissa (source of '32 *')
    return diff < 32 * std::numeric_limits<double>::epsilon();
}

//========================================================================================================================================================================================================================
// Return floating mod x % m.
//========================================================================================================================================================================================================================
double FastFMod(double x, double m)
{
    return x - (m * glm::floor(x / m));
}

//========================================================================================================================================================================================================================
// Spherical harmonics for small level l = 0,1,2,3,4
// As polynomials they are evaluated more efficiently in cartesian coordinates, assuming that d is unit.
//========================================================================================================================================================================================================================
const double FACTOR_0_0 =  0.28209479177387814347403972578038629292202531466449942842;  /*  sqrt(  1 / (  4 * pi)) */

const double FACTOR_1m1 = -0.48860251190291992158638462283834700457588560819422770213;  /* -sqrt(  3 / (  4 * pi)) */
const double FACTOR_1_0 =  0.48860251190291992158638462283834700457588560819422770213;  /*  sqrt(  3 / (  4 * pi)) */
const double FACTOR_1p1 = -0.48860251190291992158638462283834700457588560819422770213;  /* -sqrt(  3 / (  4 * pi)) */

const double FACTOR_2m2 =  1.09254843059207907054338570580268840269043295950425897535;  /*  sqrt( 15 / (  4 * pi)) */
const double FACTOR_2m1 = -1.09254843059207907054338570580268840269043295950425897535;  /* -sqrt( 15 / (  4 * pi)) */
const double FACTOR_2_0 =  0.31539156525252000603089369029571049332424750704841158784;  /*  sqrt(  5 / ( 16 * pi)) */
const double FACTOR_2p1 = -1.09254843059207907054338570580268840269043295950425897535;  /* -sqrt( 15 / (  4 * pi)) */
const double FACTOR_2p2 =  0.54627421529603953527169285290134420134521647975212948767;  /*  sqrt( 15 / ( 16 * pi)) */

const double FACTOR_3m3 = -0.59004358992664351034561027754149535000259376393696826310;  /* -sqrt( 35 / ( 32 * pi)) */
const double FACTOR_3m2 =  2.89061144264055405538938015439800900573359956132710058817;  /*  sqrt(105 / (  4 * pi)) */
const double FACTOR_3m1 = -0.45704579946446573615802069691664861528681230135886832043;  /* -sqrt( 21 / ( 32 * pi)) */
const double FACTOR_3_0 =  0.37317633259011539141439591319899726775273029669416474944;  /*  sqrt(  7 / ( 16 * pi)) */
const double FACTOR_3p1 = -0.45704579946446573615802069691664861528681230135886832043;  /* -sqrt( 21 / ( 32 * pi)) */
const double FACTOR_3p2 =  1.44530572132027702769469007719900450286679978066355029408;  /*  sqrt(105 / ( 16 * pi)) */
const double FACTOR_3p3 = -0.59004358992664351034561027754149535000259376393696826310;  /* -sqrt( 35 / ( 32 * pi)) */

const double FACTOR_4m4 =  2.50334294179670453834656209442035540394219203503734296532;  /*  sqrt(315 / ( 16 * pi)) */
const double FACTOR_4m3 = -1.77013076977993053103683083262448605000778129181090478932;  /* -sqrt(315 / ( 32 * pi)) */
const double FACTOR_4m2 =  0.94617469575756001809268107088713147997274252114523476353;  /*  sqrt( 45 / ( 16 * pi)) */
const double FACTOR_4m1 = -0.66904654355728916795211238971190597139782609521448255194;  /* -sqrt( 45 / ( 32 * pi)) */
const double FACTOR_4_0 =  0.10578554691520430380276489716764485984575949299918728566;  /*  sqrt(  9 / (256 * pi)) */
const double FACTOR_4p1 = -0.66904654355728916795211238971190597139782609521448255194;  /* -sqrt( 45 / ( 32 * pi)) */
const double FACTOR_4p2 =  0.47308734787878000904634053544356573998637126057261738176;  /*  sqrt( 45 / ( 64 * pi)) */
const double FACTOR_4p3 = -1.77013076977993053103683083262448605000778129181090478932;  /* -sqrt(315 / ( 32 * pi)) */
const double FACTOR_4p4 =  0.62583573544917613458664052360508885098554800875933574133;  /*  sqrt(315 / (256 * pi)) */

double sh_0_0(const glm::dvec3& d)                  /* 1 */
    { return FACTOR_0_0; }

double sh_1m1(const glm::dvec3& d)                  /* y */
    { return FACTOR_1m1 * d.y; }

double sh_1_0(const glm::dvec3& d)                  /* z */
    { return FACTOR_1_0 * d.z; }

double sh_1p1(const glm::dvec3& d)                  /* x */
    { return FACTOR_1p1 * d.x; }

double sh_2m2(const glm::dvec3& d)                  /* xy */
    { return FACTOR_2m2 * d.x * d.y; }

double sh_2m1(const glm::dvec3& d)                  /* yz */
    { return FACTOR_2m1 * d.y * d.z; }

double sh_2_0(const glm::dvec3& d)                  /* -xx - yy + 2zz */
    { return FACTOR_2_0 * (-d.x * d.x - d.y * d.y + 2.0 * d.z * d.z); }

double sh_2p1(const glm::dvec3& d)                  /* xz */
    { return FACTOR_2p1 * d.x * d.z; }

double sh_2p2(const glm::dvec3& d)                  /* xx - yy */
    { return FACTOR_2p2 * (d.x * d.x - d.y * d.y); }

double sh_3m3(const glm::dvec3& d)                  /* y * (3xx - yy) */
    { return FACTOR_3m3 * d.y * (3.0 * d.x * d.x - d.y * d.y); }

double sh_3m2(const glm::dvec3& d)                  /* xyz */
    { return FACTOR_3m2 * d.x * d.y * d.z; }

double sh_3m1(const glm::dvec3& d)                  /* y * (4zz - xx - yy) */
    { return FACTOR_3m1 * d.y * (4.0 * d.z * d.z - d.x * d.x - d.y * d.y); }

double sh_3_0(const glm::dvec3& d)                  /* z * (2zz - 3xx - 3yy) */
    { return FACTOR_3_0 * d.z * (2.0 * d.z * d.z - 3.0 * d.x * d.x - 3.0 * d.y * d.y); }

double sh_3p1(const glm::dvec3& d)                  /* x * (4zz - xx - yy) */
    { return FACTOR_3p1 * d.x * (4.0 * d.z * d.z - d.x * d.x - d.y * d.y); }

double sh_3p2(const glm::dvec3& d)                  /* z * (xx - yy) */
    { return FACTOR_3p2 * d.z * (d.x * d.x - d.y * d.y); }

double sh_3p3(const glm::dvec3& d)                  /* x * (xx - 3yy) */
    { return FACTOR_3p3 * d.x * (d.x * d.x - 3.0 * d.y * d.y); }

double sh_4m4(const glm::dvec3& d)                  /* xy * (xx - yy) */
    { return FACTOR_4m4 * d.x * d.y * (d.x * d.x - d.y * d.y); }

double sh_4m3(const glm::dvec3& d)                  /* yz * (3xx - yy) */
    { return FACTOR_4m3 * d.y * d.z * (3.0 * d.x * d.x - d.y * d.y); }

double sh_4m2(const glm::dvec3& d)                  /* xy * (7zz - 1) */
    { return FACTOR_4m2 * d.x * d.y * (7.0 * d.z * d.z - 1.0); }

double sh_4m1(const glm::dvec3& d)                  /* yz * (7zz - 3) */
    { return FACTOR_4m1 * d.y * d.z * (7.0 * d.z * d.z - 3.0); }

double sh_4_0(const glm::dvec3& d)                  /* (35zz - 30) * zz + 3 */
{
    double zz = d.z * d.z;
    return FACTOR_4_0 * ((35.0 * zz - 30.0) * zz + 3.0);
}

double sh_4p1(const glm::dvec3& d)                  /* xz * (7zz - 3) */
    { return FACTOR_4p1 * d.x * d.z * (7.0 * d.z * d.z - 3.0); }

double sh_4p2(const glm::dvec3& d)                  /* (xx - yy) * (7zz - 1) */
    { return FACTOR_4p2 * (d.x * d.x - d.y * d.y) * (7.0 * d.z * d.z - 1.0); }

double sh_4p3(const glm::dvec3& d)                  /* xz * (xx - 3yy) */
    { return FACTOR_4p3 * d.x * d.z * (d.x * d.x - 3.0 * d.y * d.y); }

double sh_4p4(const glm::dvec3& d)                  /* (xx * (xx - 3yy) - yy * (3xx - yy)) */
{
    double xx = d.x * d.x;
    double yy = d.y * d.y;
    return FACTOR_4p4 * (xx * (xx - 3.0 * yy) - yy * (3.0 * xx - yy));
}

//========================================================================================================================================================================================================================
// Compute the factorial for an integer x. It is assumed x >= 0.
// This implementation precomputes the results for low values of x, in which case this is a constant time lookup.
// The vast majority of SH evaluations will hit these precomputed values.
//========================================================================================================================================================================================================================
double Factorial(int x)
{
    const double factorial_cache[kCacheSize] = {1.0, 1.0, 2.0, 6.0, 24.0, 120.0, 720.0, 5040.0, 40320.0, 362880.0, 3628800.0, 39916800.0, 479001600.0, 6227020800.0, 87178291200.0, 1307674368000.0};

    if (x < kCacheSize)
        return factorial_cache[x];

    double s = 1.0;
    for (int n = 2; n <= x; n++)
        s *= n;
    return s;
}

//========================================================================================================================================================================================================================
// Compute the double factorial for an integer x. This assumes x >= 0.
// This implementation precomputes the results for low values of x, in which case this is a constant time lookup.
// The vast majority of SH evaluations will hit these precomputed values.
//========================================================================================================================================================================================================================
double DoubleFactorial(int x)
{
    const double dbl_factorial_cache[kCacheSize] = {1.0, 1.0, 2.0, 3.0, 8.0, 15.0, 48.0, 105.0, 384.0, 945.0, 3840.0, 10395.0, 46080.0, 135135.0, 645120.0, 2027025.0};

    if (x < kCacheSize)
        return dbl_factorial_cache[x];

    double s = 1.0;
    double n = x;
    while (n > 1.0)
    {
        s *= n;
        n -= 2.0;
    }
    return s;
}

//========================================================================================================================================================================================================================
// Evaluate the associated Legendre polynomial of degree l and order m at coordinate x. The inputs must satisfy:
// 1. l >= 0
// 2. 0 <= m <= l
// 3. -1 <= x <= 1
//
// This implementation is based off the approach described in [1], instead of computing Pml(x) directly, Pmm(x) is computed.
// Pmm can be lifted to Pmm+1 recursively until Pml is found
//========================================================================================================================================================================================================================
double EvalLegendrePolynomial(int l, int m, double x)
{
    double pmm = 1.0;                           /* compute Pmm(x) = (-1)^m(2m - 1)!!(1 - x^2)^(m/2) */

    if (m > 0)                                  /* P00 is defined as 1.0, do don't evaluate Pmm unless we know m > 0 */
    {
        double sign = (m % 2 == 0 ? 1 : -1);
        pmm = sign * DoubleFactorial(2 * m - 1) * glm::pow(1 - x * x, m / 2.0);
    }

    if (l == m)                                 /* Pml is the same as Pmm so there's no lifting to higher bands needed */
        return pmm;


    double pmm1 = x * (2 * m + 1) * pmm;        /* Compute Pmm+1(x) = x(2m + 1)Pmm(x)*/

    if (l == m + 1)                             /* Pml is the same as Pmm+1 so we are done as well */
        return pmm1;

    for (int n = m + 2; n <= l; n++)            /* P{m, l}(x) = ((2l - 1)xP{m, l - 1}(x) - (l + m - 1)P{m, l - 2}(x)) / (l - m) */
    {
        double pmn = (x * (2 * n - 1) * pmm1 - (n + m - 1) * pmm) / (n - m);
        pmm = pmm1;
        pmm1 = pmn;
    }

    return pmm1;          /* Pmm1 at the end of the above loop is equal to Pml */
}

//========================================================================================================================================================================================================================
// The following functions are used to implement SH rotation computations based on the recursive approach described in [1, 4].
// The names of the functions correspond with the notation used in [1, 4].
//========================================================================================================================================================================================================================
double KroneckerDelta(int i, int j)
{
    if (i == j)
        return 1.0;
    return 0.0;
}

//========================================================================================================================================================================================================================
// [4] uses an odd convention of referring to the rows and columns using centered indices, so the middle row and column are (0, 0) and the upper left would have negative coordinates.
// This is a convenience function to allow us to access an Eigen::MatrixXd in the same manner, assuming r is a (2l+1)x(2l+1) matrix.
//========================================================================================================================================================================================================================
double GetCenteredElement(const matrix& r, int i, int j)
{
    // The shift to go from [-l, l] to [0, 2l] is (rows - 1) / 2 = l,
    // (since the matrix is assumed to be square, rows == cols).
    int offset = (r.rows() - 1) / 2;
    return r(i + offset, j + offset);
}

//========================================================================================================================================================================================================================
// P is a helper function defined in [4] that is used by the functions U, V, W.
// This should not be called on its own, as U, V, and W (and their coefficients)
// select the appropriate matrix elements to access (arguments @a and @b).
//========================================================================================================================================================================================================================
double P(int i, int a, int b, int l, const std::vector<matrix>& r)
{
    if (b == l)
        return GetCenteredElement(r[1], i, 1) * GetCenteredElement(r[l - 1], a, l - 1) - GetCenteredElement(r[1], i, -1) * GetCenteredElement(r[l - 1], a, -l + 1);
    if (b == -l)
        return GetCenteredElement(r[1], i, 1) * GetCenteredElement(r[l - 1], a, -l + 1) + GetCenteredElement(r[1], i, -1) * GetCenteredElement(r[l - 1], a, l - 1);

    return GetCenteredElement(r[1], i, 0) * GetCenteredElement(r[l - 1], a, b);
}

//========================================================================================================================================================================================================================
// The functions U, V, and W should only be called if the correspondingly named coefficient u, v, w from the function ComputeUVWCoeff() is non-zero.
// When the coefficient is 0, these would attempt to access matrix elements that are out of bounds.
// The list of rotations, r, must have the l - 1 previously completed band rotations. These functions are valid for l >= 2.
//========================================================================================================================================================================================================================
double U(int m, int n, int l, const std::vector<matrix>& r)
{
    //====================================================================================================================================================================================================================
    // Although [1, 4] split U into three cases for m == 0, m < 0, m > 0 the actual values are the same for all three cases
    //====================================================================================================================================================================================================================
    return P(0, m, n, l, r);
}

double V(int m, int n, int l, const std::vector<matrix>& r)
{
    if (m == 0) {
        return P(1, 1, n, l, r) + P(-1, -1, n, l, r);
    }
    else if (m > 0)
    {
        return P(1, m - 1, n, l, r) * sqrt(1 + KroneckerDelta(m, 1)) - P(-1, -m + 1, n, l, r) * (1 - KroneckerDelta(m, 1));
    }
    else
    {
        //================================================================================================================================================================================================================
        // Note there is apparent errata in [1,4,4b] dealing with this particular case.
        // [4b] writes it should be P * (1 - d) + P * (1 - d) ^ 0.5
        // [1] writes it as P * (1 + d) + P * (1 - d) ^ 0.5, but going through the math by hand,
        // you must have it as P * (1 - d) + P * (1 + d) ^ 0.5 to form a 2^0.5 term, which parallels the case where m > 0.
        //================================================================================================================================================================================================================
        return P(1, m + 1, n, l, r) * (1 - KroneckerDelta(m, -1)) + P(-1, -m - 1, n, l, r) * sqrt(1 + KroneckerDelta(m, -1));
    }
}

double W(int m, int n, int l, const std::vector<matrix>& r)
{
    if (m == 0)
        return 0.0;
        //================================================================================================================================================================================================================
        // whenever this happens, w is also 0 so W can be anything
        //================================================================================================================================================================================================================
    if (m > 0)
        return P(1, m + 1, n, l, r) + P(-1, -m - 1, n, l, r);
    return P(1, m - 1, n, l, r) - P(-1, -m + 1, n, l, r);
}

//========================================================================================================================================================================================================================
// Calculate the coefficients applied to the U, V, and W functions. Because
// their equations share many common terms they are computed simultaneously.
//========================================================================================================================================================================================================================
void ComputeUVWCoeff(int m, int n, int l, double* u, double* v, double* w)
{
    double d = KroneckerDelta(m, 0);
    double denom = (abs(n) == l ? 2.0 * l * (2.0 * l - 1) : (l + n) * (l - n));

    *u = sqrt((l + m) * (l - m) / denom);
    *v = 0.5 * sqrt((1 + d) * (l + abs(m) - 1.0) * (l + abs(m)) / denom) * (1 - 2 * d);
    *w = -0.5 * sqrt((l - abs(m) - 1) * (l - abs(m)) / denom) * (1 - d);
}

//========================================================================================================================================================================================================================
// Calculate the (2l+1)x(2l+1) rotation matrix for the band @l.
// This uses the matrices computed for band 1 and band l-1 to compute the
// matrix for band l. @rotations must contain the previously computed l-1
// rotation matrices, and the new matrix for band l will be appended to it.
//
// This implementation comes from p. 5 (6346), Table 1 and 2 in [4] taking into account the corrections from [4b].
//========================================================================================================================================================================================================================
void ComputeBandRotation(int l, std::vector<matrix>* rotations)
{
    // The band's rotation matrix has rows and columns equal to the number of
    // coefficients within that band (-l <= m <= l implies 2l + 1 coefficients).
    matrix rotation(2 * l + 1, 2 * l + 1);
    for (int m = -l; m <= l; m++)
    {
        for (int n = -l; n <= l; n++)
        {
            double u, v, w;
            ComputeUVWCoeff(m, n, l, &u, &v, &w);

            // The functions U, V, W are only safe to call if the coefficients u, v, w are not zero
            if (!NearByMargin(u, 0.0))
                u *= U(m, n, l, *rotations);
            if (!NearByMargin(v, 0.0))
                v *= V(m, n, l, *rotations);
            if (!NearByMargin(w, 0.0))
                w *= W(m, n, l, *rotations);

            rotation(m + l, n + l) = (u + v + w);
        }
    }

    rotations->push_back(rotation);
}

}  // namespace

glm::dvec3 ToVector(double phi, double theta)
{
    double r = glm::sin(theta);
    return glm::dvec3(r * glm::cos(phi), r * glm::sin(phi), glm::cos(theta));
}

void ToSphericalCoords(const glm::dvec3& dir, double* phi, double* theta)
{
    assert(NearByMargin(glm::length2(dir), 1.0) && "dir is not unit");
    //====================================================================================================================================================================================================================
    // Explicitly clamp the z coordinate so that numeric errors don't cause it to fall just outside of acos' domain.
    //====================================================================================================================================================================================================================
    *theta = glm::acos(glm::clamp(dir.z, -1.0, 1.0));
    //====================================================================================================================================================================================================================
    // We don't need to divide dir.y() or dir.x() by sin(theta) since they are both scaled by it and atan2 will handle it appropriately.
    //====================================================================================================================================================================================================================
    *phi = atan2(dir.y, dir.x);
}

double ImageXToPhi(int x, int res_x)
{
    //====================================================================================================================================================================================================================
    // The directions are measured from the center of the pixel, so add 0.5
    // to convert from integer pixel indices to float pixel coordinates.
    //====================================================================================================================================================================================================================
    return constants::two_pi_d * (x + 0.5) / res_x;
}

double ImageYToTheta(int y, int height)
{
    return constants::pi_d * (y + 0.5) / height;
}

glm::dvec2 ToImageCoords(double phi, double theta, int width, int height)
{
    //====================================================================================================================================================================================================================
    // Allow theta to repeat and map to 0 to pi. However, to account for cases
    // where y goes beyond the normal 0 to pi range, phi may need to be adjusted.
    //====================================================================================================================================================================================================================
    theta = Clamp(FastFMod(theta, constants::two_pi_d), 0.0, constants::two_pi_d);
    if (theta > constants::pi_d)
    {
        //================================================================================================================================================================================================================
        // theta is out of bounds. Effectively, theta has rotated past the pole so after adjusting theta to be in range,
        // rotating phi by pi forms an equivalent direction.
        //================================================================================================================================================================================================================
        theta = constants::two_pi_d - theta;  // now theta is between 0 and pi
        phi += constants::pi_d;
    }
    //====================================================================================================================================================================================================================
    // Allow phi to repeat and map to the normal 0 to 2pi range.
    // Clamp and map after adjusting theta in case theta was forced to update phi.
    //====================================================================================================================================================================================================================
      phi = Clamp(FastFMod(phi, 2.0 * M_PI), 0.0, 2.0 * M_PI);

    //====================================================================================================================================================================================================================
    // Now phi is in [0, 2pi] and theta is in [0, pi] so it's simple to inverse the linear equations in ImageCoordsToSphericalCoords, although there's no
    // -0.5 because we're returning floating point coordinates and so don't need to center the pixel.
    //====================================================================================================================================================================================================================
    return glm::dvec2(width * phi / (constants::two_pi_d), height * theta / constants::pi_d);
}

double EvalSHSlow(int l, int m, double phi, double theta)
{
    assert((l >= 0) && "l must be at least 0.");
    assert((-l <= m && m <= l) && "m must be between -l and l.");

    double kml = glm::sqrt((2.0 * l + 1) * Factorial(l - glm::abs(m)) / (4.0 * M_PI * Factorial(l + abs(m))));
    if (m > 0)
        return glm::sqrt(2.0) * kml * glm::cos(m * phi) * EvalLegendrePolynomial(l, m, cos(theta));
    if (m < 0)
        return glm::sqrt(2.0) * kml * glm::sin(-m * phi) * EvalLegendrePolynomial(l, -m, cos(theta));

    return kml * EvalLegendrePolynomial(l, 0, cos(theta));
}

double EvalSHSlow(int l, int m, const glm::dvec3& dir)
{
    double phi, theta;
    ToSphericalCoords(dir, &phi, &theta);
    return EvalSH(l, m, phi, theta);
}

double EvalSH(int l, int m, double phi, double theta)
{
    if (l <= kHardCodedOrderLimit)                              /* If using the hardcoded functions, switch to cartesian */
        return EvalSH(l, m, ToVector(phi, theta));
    return EvalSHSlow(l, m, phi, theta);                        /* Otherwise, stay in spherical coordinates since that's what the recurrence version is implemented in */
}

double EvalSH(int l, int m, const glm::dvec3& dir)
{
    if (l <= kHardCodedOrderLimit)
    {
        //================================================================================================================================================================================================================
        // Validate l and m here (don't do it generally since EvalSHSlow also
        // checks it if we delegate to that function).
        //================================================================================================================================================================================================================
        assert((l >= 0) && "l must be at least 0.");
        assert((-l <= m && m <= l) && "m must be between -l and l.");

        if (l == 0)
            return sh_0_0(dir);
        if (l == 1)
        {
            if (m == -1) return sh_1m1(dir);
            if (m ==  0) return sh_1_0(dir);
            return sh_1p1(dir);
        }
        if (l == 2)
        {
            if (m == -2) return sh_2m2(dir);
            if (m == -1) return sh_2m1(dir);
            if (m ==  0) return sh_2_0(dir);
            if (m ==  1) return sh_2p1(dir);
            return sh_2p2(dir);
        }

        if (l == 3)
        {
            if (m == -3) return sh_3m3(dir);
            if (m == -2) return sh_3m2(dir);
            if (m == -1) return sh_3m1(dir);
            if (m ==  0) return sh_3_0(dir);
            if (m ==  1) return sh_3p1(dir);
            if (m ==  2) return sh_3p2(dir);
            return sh_3p3(dir);
        }
        if (l == 4)
        {
            if (m == -4) return sh_4m4(dir);
            if (m == -3) return sh_4m3(dir);
            if (m == -2) return sh_4m2(dir);
            if (m ==  1) return sh_4m1(dir);
            if (m ==  0) return sh_4_0(dir);
            if (m ==  1) return sh_4p1(dir);
            if (m ==  2) return sh_4p2(dir);
            if (m ==  3) return sh_4p3(dir);
            return sh_4p4(dir);
        }
        return 0.0;         /* unreachable, but makes compiler happy */
    }
    /* Not hard-coded so use the recurrence relation (which will convert this to spherical coordinates) */
    return EvalSHSlow(l, m, dir);
}

std::unique_ptr<std::vector<double>> ProjectFunction(int order, const SphericalFunction& func, int sample_count)
{
    assert((order >= 0) && "Order must be at least zero.");
    assert((sample_count > 0) && "Sample count must be at least one.");

    // This is the approach demonstrated in [1] and is useful for arbitrary
    // functions on the sphere that are represented analytically.
    const int sample_side = static_cast<int>(floor(sqrt(sample_count)));
    std::unique_ptr<std::vector<double>> coeffs(new std::vector<double>());
    coeffs->assign(GetCoefficientCount(order), 0.0);

    // generate sample_side^2 uniformly and stratified samples over the sphere
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> rng(0.0, 1.0);
    for (int t = 0; t < sample_side; t++)
    {
        for (int p = 0; p < sample_side; p++)
        {
            double alpha = (t + rng(gen)) / sample_side;
            double beta = (p + rng(gen)) / sample_side;

            //============================================================================================================================================================================================================
            // See http://www.bogotobogo.com/Algorithms/uniform_distribution_sphere.php
            //============================================================================================================================================================================================================
            double phi = constants::two_pi_d * beta;
            double theta = acos(2.0 * alpha - 1.0);

            //============================================================================================================================================================================================================
            // evaluate the analytic function for the current spherical coords
            //============================================================================================================================================================================================================
            double func_value = func(phi, theta);

            //============================================================================================================================================================================================================
            // evaluate the SH basis functions up to band O, scale them by the
            // function's value and accumulate them over all generated samples
            //============================================================================================================================================================================================================
            for (int l = 0; l <= order; l++)
            {
                for (int m = -l; m <= l; m++)
                {
                    double sh = EvalSH(l, m, phi, theta);
                    (*coeffs)[GetIndex(l, m)] += func_value * sh;
                }
            }
        }
    }

    //====================================================================================================================================================================================================================
    // scale by the probability of a particular sample, which is 4pi/sample_side^2.
    // 4pi for the surface area of a unit sphere, and 1/sample_side^2 for the number of samples drawn uniformly.
    //====================================================================================================================================================================================================================
    double weight = 2.0 * constants::two_pi_d / (sample_side * sample_side);
    for (unsigned int i = 0; i < coeffs->size(); i++)
    {
        (*coeffs)[i] *= weight;
    }

    return coeffs;
}

std::unique_ptr<std::vector<Eigen::Array3f>> ProjectEnvironment(int order, const Image& env)
{
    assert((order >= 0) && "Order must be at least zero.");

    //====================================================================================================================================================================================================================
    // An environment map projection is three different spherical functions, one for each color channel.
    // The projection integrals are estimated by iterating over every pixel within the image.
    //====================================================================================================================================================================================================================
    double pixel_area = (constants::two_pi_d / env.width()) * (constants::pi_d / env.height());

    std::unique_ptr<std::vector<Eigen::Array3f>> coeffs(new std::vector<Eigen::Array3f>());
    coeffs->assign(GetCoefficientCount(order), Eigen::Array3f(0.0, 0.0, 0.0));

    glm::vec3 color;
    for (int t = 0; t < env.height(); t++)
    {
        double theta = ImageYToTheta(t, env.height());
        //================================================================================================================================================================================================================
        // The differential area of each pixel in the map is constant across a
        // row. Must scale the pixel_area by sin(theta) to account for the
        // stretching that occurs at the poles with this parameterization.
        //================================================================================================================================================================================================================
        double weight = pixel_area * sin(theta);

        for (int p = 0; p < env.width(); p++)
        {
            double phi = ImageXToPhi(p, env.width());
            color = env.GetPixel(p, t);

            for (int l = 0; l <= order; l++)
            {
                for (int m = -l; m <= l; m++)
                {
                    int i = GetIndex(l, m);
                    double sh = EvalSH(l, m, phi, theta);
                    (*coeffs)[i] += sh * weight * color.array();
                }
            }
        }
    }

    return coeffs;
}

std::unique_ptr<std::vector<double>> ProjectSparseSamples(int order, const std::vector<Eigen::Vector3d>& dirs, const std::vector<double>& values)
{
    assert((order >= 0) && "Order must be at least zero.");
    assert((dirs.size() == values.size()) && "Directions and values must have the same size.");

    //====================================================================================================================================================================================================================
    // Solve a linear least squares system Ax = b for the coefficients, x.
    // Each row in the matrix A are the values of the spherical harmonic basis
    // functions evaluated at that sample's direction (from @dirs). The
    // corresponding row in b is the value in @values.
    //====================================================================================================================================================================================================================
    std::unique_ptr<std::vector<double>> coeffs(new std::vector<double>());
    coeffs->assign(GetCoefficientCount(order), 0.0);

    matrix basis_values(dirs.size(), coeffs->size());
    matrix func_values(dirs.size());

    double phi, theta;
    for (unsigned int i = 0; i < dirs.size(); i++)
    {
        func_values(i) = values[i];
        ToSphericalCoords(dirs[i], &phi, &theta);

        for (int l = 0; l <= order; l++)
        {
            for (int m = -l; m <= l; m++)
            {
                basis_values(i, GetIndex(l, m)) = EvalSH(l, m, phi, theta);
            }
        }
    }

    //====================================================================================================================================================================================================================
    // Use SVD to find the least squares fit for the coefficients of the basis functions that best match the data
    //====================================================================================================================================================================================================================
    Eigen::VectorXd soln = basis_values.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(func_values);

    //====================================================================================================================================================================================================================
    // Copy everything over to our coeffs array
    //====================================================================================================================================================================================================================
    for (unsigned int i = 0; i < coeffs->size(); i++)
        (*coeffs)[i] = soln(i);
    return coeffs;
}

template <typename T> T EvalSHSum(int order, const std::vector<T>& coeffs, double phi, double theta)
{
    if (order <= kHardCodedOrderLimit)
    {
        //================================================================================================================================================================================================================
        // It is faster to compute the cartesian coordinates once
        //================================================================================================================================================================================================================
        return EvalSHSum(order, coeffs, ToVector(phi, theta));
    }

    assert((GetCoefficientCount(order) == coeffs.size()) && "Incorrect number of coefficients provided.");
    T sum = Zero<T>();
    for (int l = 0; l <= order; l++)
    {
        for (int m = -l; m <= l; m++)
        {
            sum += EvalSH(l, m, phi, theta) * coeffs[GetIndex(l, m)];
        }
    }
    return sum;
}

template <typename T> T EvalSHSum(int order, const std::vector<T>& coeffs, const Eigen::Vector3d& dir)
{
    if (order > kHardCodedOrderLimit)
    {
        //================================================================================================================================================================================================================
        // It is faster to switch to spherical coordinates
        //================================================================================================================================================================================================================
        double phi, theta;
        ToSphericalCoords(dir, &phi, &theta);
        return EvalSHSum(order, coeffs, phi, theta);
    }

    assert((GetCoefficientCount(order) == coeffs.size()) && "Incorrect number of coefficients provided.");
    assert((NearByMargin(dir.squaredNorm(), 1.0) && "dir is not unit.");

    T sum = Zero<T>();
    for (int l = 0; l <= order; l++)
    {
        for (int m = -l; m <= l; m++)
        {
            sum += EvalSH(l, m, dir) * coeffs[GetIndex(l, m)];
        }
    }
    return sum;
}

Rotation::Rotation(int order, const Eigen::Quaterniond& rotation)
    : order_(order), rotation_(rotation)
{
    band_rotations_.reserve(GetCoefficientCount(order));
}

std::unique_ptr<Rotation> Rotation::Create(int order, const Eigen::Quaterniond& rotation)
{
    assert((order >= 0) && "Order must be at least 0.");
    assert((NearByMargin(rotation.squaredNorm(), 1.0) && "Rotation must be normalized.");

    std::unique_ptr<Rotation> sh_rot(new Rotation(order, rotation));

    //====================================================================================================================================================================================================================
    // Order 0 (first band) is simply the 1x1 identity since the SH basis function is a simple sphere.
    //====================================================================================================================================================================================================================
    matrix r(1, 1);
    r(0, 0) = 1.0;
    sh_rot->band_rotations_.push_back(r);

    r.resize(3, 3);

    //====================================================================================================================================================================================================================
    // The second band's transformation is simply a permutation of the
    // rotation matrix's elements, provided in Appendix 1 of [1], updated to
    // include the Condon-Shortely phase. The recursive method in
    // ComputeBandRotation preserves the proper phases as high bands are computed.
    //====================================================================================================================================================================================================================

    Eigen::Matrix3d rotation_mat = rotation.toRotationMatrix();
    r(0, 0) =  rotation_mat(1, 1);
    r(0, 1) = -rotation_mat(1, 2);
    r(0, 2) =  rotation_mat(1, 0);
    r(1, 0) = -rotation_mat(2, 1);
    r(1, 1) =  rotation_mat(2, 2);
    r(1, 2) = -rotation_mat(2, 0);
    r(2, 0) =  rotation_mat(0, 1);
    r(2, 1) = -rotation_mat(0, 2);
    r(2, 2) =  rotation_mat(0, 0);
    sh_rot->band_rotations_.push_back(r);

    //====================================================================================================================================================================================================================
    // Recursively build the remaining band rotations, using the equations provided in [4, 4b].
    //====================================================================================================================================================================================================================
    for (int l = 2; l <= order; l++)
        ComputeBandRotation(l, &(sh_rot->band_rotations_));

    return sh_rot;
}

std::unique_ptr<Rotation> Rotation::Create(int order, const Rotation& rotation)
{
    assert((order >= 0) && "Order must be at least 0.");
    std::unique_ptr<Rotation> sh_rot(new Rotation(order, rotation.rotation_));

    //====================================================================================================================================================================================================================
    // Copy up to min(order, rotation.order_) band rotations into the new SHRotation.
    // For shared orders, they are the same. If the new order is higher than already calculated then the remainder will be computed next.
    //====================================================================================================================================================================================================================
    for (int l = 0; l <= std::min(order, rotation.order_); l++)
        sh_rot->band_rotations_.push_back(rotation.band_rotations_[l]);

    //====================================================================================================================================================================================================================
    // Calculate remaining bands (automatically skipped if there are no more).
    //====================================================================================================================================================================================================================
    for (int l = rotation.order_ + 1; l <= order; l++)
        ComputeBandRotation(l, &(sh_rot->band_rotations_));

    return sh_rot;
}

int Rotation::order() const
    { return order_; }

glm::dquat Rotation::rotation() const
    { return rotation_; }

const matrix& Rotation::band_rotation(int l) const
{
    return band_rotations_[l];
}

template <typename T> void Rotation::Apply(const std::vector<T>& coeff, std::vector<T>* result) const
{
    assert((coeff.size() == GetCoefficientCount(order_)) && "Incorrect number of coefficients provided.");

    //====================================================================================================================================================================================================================
    // Resize to the required number of coefficients.
    // If result is already the same size as coeff, there's no need to zero out its values since each index will be written explicitly later.
    //====================================================================================================================================================================================================================
    if (result->size() != coeff.size())
        result->assign(coeff.size(), T());

    //====================================================================================================================================================================================================================
    // Because of orthogonality, the coefficients outside of each band do not
    // interact with one another. By separating them into band-specific matrices, we take advantage of that sparsity.
    //====================================================================================================================================================================================================================
    for (int l = 0; l <= order_; l++)
    {
        VectorX<T> band_coeff(2 * l + 1);

        //================================================================================================================================================================================================================
        // Fill band_coeff from the subset of @coeff that's relevant.
        // Offset by l to get the appropiate vector component (0-based instead of starting at -l).
        //================================================================================================================================================================================================================
        for (int m = -l; m <= l; m++)
            band_coeff(m + l) = coeff[GetIndex(l, m)];

        band_coeff = band_rotations_[l].cast<T>() * band_coeff;

        //================================================================================================================================================================================================================
        // Copy rotated coefficients back into the appropriate subset into @result.
        //================================================================================================================================================================================================================
        for (int m = -l; m <= l; m++)
            (*result)[GetIndex(l, m)] = band_coeff(m + l);
    }
}

void RenderDiffuseIrradianceMap(const Image& env_map, Image* diffuse_out)
{
    std::unique_ptr<std::vector<Eigen::Array3f>> coeffs = ProjectEnvironment(kIrradianceOrder, env_map);
    RenderDiffuseIrradianceMap(*coeffs, diffuse_out);
}

void RenderDiffuseIrradianceMap(const std::vector<Eigen::Array3f>& sh_coeffs, Image* diffuse_out)
{
    for (int y = 0; y < diffuse_out->height(); y++)
    {
        double theta = ImageYToTheta(y, diffuse_out->height());
        for (int x = 0; x < diffuse_out->width(); x++)
        {
            double phi = ImageXToPhi(x, diffuse_out->width());
            glm::dvec3 normal = ToVector(phi, theta);
            glm::vec3 irradiance = RenderDiffuseIrradiance(sh_coeffs, normal);
            diffuse_out->SetPixel(x, y, irradiance);
        }
    }
}

glm::vec3 RenderDiffuseIrradiance(const std::vector<glm::vec3>& sh_coeffs, const glm::dvec3& normal)
{
    //====================================================================================================================================================================================================================
    // Optimization for if sh_coeffs is empty, then there is no environmental illumination so irradiance is 0.0 regardless of the normal.
    //====================================================================================================================================================================================================================
    if (sh_coeffs.empty())
        return glm::vec3(0.0);

    //====================================================================================================================================================================================================================
    // Compute diffuse irradiance
    //====================================================================================================================================================================================================================
    Eigen::Quaterniond rotation;
    rotation.setFromTwoVectors(Eigen::Vector3d::UnitZ(), normal).normalize();

    std::vector<double> rotated_cos(kIrradianceCoeffCount);
    std::unique_ptr<sh::Rotation> sh_rot(Rotation::Create(kIrradianceOrder, rotation));
    sh_rot->Apply(cosine_lobe, &rotated_cos);

    glm::vec3 sum(0.0f);
    //====================================================================================================================================================================================================================
    // The cosine lobe is 9 coefficients and after that all bands are assumed to be 0.
    // If sh_coeffs provides more than 9, they are irrelevant then. If it provides fewer than 9,
    // this assumes that the remaining coefficients would have been 0 and can safely ignore the rest of the cosine lobe.
    //====================================================================================================================================================================================================================
    unsigned int coeff_count = kIrradianceCoeffCount;
    if (coeff_count > sh_coeffs.size())
        coeff_count = sh_coeffs.size();
    for (unsigned int i = 0; i < coeff_count; i++)
        sum += rotated_cos[i] * sh_coeffs[i];
    return sum;
}

//========================================================================================================================================================================================================================
// Template specializations
//========================================================================================================================================================================================================================
template double EvalSHSum<double>(int order, const std::vector<double>& coeffs, double phi, double theta);
template double EvalSHSum<double>(int order, const std::vector<double>& coeffs, const Eigen::Vector3d& dir);

template float EvalSHSum<float>(int order, const std::vector<float>& coeffs, double phi, double theta);
template float EvalSHSum<float>(int order, const std::vector<float>& coeffs, const Eigen::Vector3d& dir);

template Eigen::Array3f EvalSHSum<Eigen::Array3f>(int order,  const std::vector<Eigen::Array3f>& coeffs, double phi, double theta);
template Eigen::Array3f EvalSHSum<Eigen::Array3f>(int order,  const std::vector<Eigen::Array3f>& coeffs, const Eigen::Vector3d& dir);

template void Rotation::Apply<double>(const std::vector<double>& coeff, std::vector<double>* result) const;
template void Rotation::Apply<float>(const std::vector<float>& coeff, std::vector<float>* result) const;

//========================================================================================================================================================================================================================
// The generic implementation for Rotate doesn't handle aggregate types
// like Array3f so split it apart, use the generic version and then recombine
// them into the final result.
//========================================================================================================================================================================================================================
template <> void Rotation::Apply<Eigen::Array3f>(const std::vector<Eigen::Array3f>& coeff, std::vector<Eigen::Array3f>* result) const
{
    // Separate the Array3f coefficients into three vectors.
    std::vector<float> c1, c2, c3;
    for (unsigned int i = 0; i < coeff.size(); i++)
    {
        const Eigen::Array3f& c = coeff[i];
        c1.push_back(c(0));
        c2.push_back(c(1));
        c3.push_back(c(2));
    }

    // Compute the rotation in place
    Apply(c1, &c1);
    Apply(c2, &c2);
    Apply(c3, &c3);

    // Coellesce back into Array3f
    result->assign(GetCoefficientCount(order_), Eigen::Array3f::Zero());
    for (unsigned int i = 0; i < result->size(); i++)
    {
        (*result)[i] = Eigen::Array3f(c1[i], c2[i], c3[i]);
    }
}

}  // namespace sh
