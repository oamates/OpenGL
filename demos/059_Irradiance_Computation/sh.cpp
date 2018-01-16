
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
// Spherical harmonic sampler :: basic structure for fast coefficient calculations
//========================================================================================================================================================================================================================
struct sh_sampler_t
{
    struct sample_t
    {
        glm::dvec3 spherical;
        glm::dvec3 cartesian;
    };

    int N;
    int L;
    sample_t* samples;
    double* values;

    /* constructs N x N uniformly distributed spherical point cloud and matrix of
       precomputed values of spherical functions of level < L using jittered stratification */

    sh_sampler_t(int N, int L) : N(N), L(L)
    {
        size_t buf_size = N * N * (sizeof(sample_t) + L * L * sizeof(double));
        samples = (sample_t*) malloc(buf_size);
        values  = (double*) (samples + N * N);

        int i = 0;              // spherical point array index
        int v = 0;              // values array index
        double inv_N = 1.0 / N;

        for(int a = 0; a < N; a++)
        {
            for(int b = 0; b < N; b++)
            {
                // generate unbiased distribution of spherical coords

                double x = (a + random()) * inv_N;
                double y = (b + random()) * inv_N;

                double phi = constants::two_pi_d * x;
                double theta = 2.0 * glm::acos(glm::sqrt(1.0 - y));

                samples[i].spherical   = glm::dvec3(phi, theta, 1.0);
                double sin_theta = glm::sin(theta);
                double cos_theta = glm::cos(theta);
                samples[i].cartesian = glm::dvec3(sin_theta * glm::cos(phi), sin_theta * glm::sin(phi), cos_theta);

                for(int l = 0; l < L; ++l)
                {
                    for(int m = -l; m <= l; ++m)
                    {
                        values[v++] = sh(l, m, theta, phi);
                    }
                }
                ++i;
            }
        }
    }

    ~sh_sampler_t()
        { free(samples); }
};


//========================================================================================================================================================================================================================
// Associated Legendre polynomial P(l, m, x) at x
//========================================================================================================================================================================================================================
double Legendre(int l, int m, double x)
{
    double pmm = 1.0;

    if (m > 0)
    {
        double somx2 = glm::sqrt((1.0 - x) * (1.0 + x));
        double fact = 1.0;
        for(int i = 1; i <= m; i++)
        {
            pmm *= (-fact) * somx2;
            fact += 2.0;
        }
    }

    if (l == m)
        return pmm;
    double pmmp1 = x * (2.0 * m + 1.0) * pmm;

    if (l == m + 1)
        return pmmp1;
    double pll = 0.0;
    for(int ll = m + 2; ll <= l; ++ll)
    {
        pll = ( (2.0 * ll - 1.0) * x * pmmp1 - (ll + m - 1.0) * pmm) / (ll - m);
        pmm = pmmp1;
        pmmp1 = pll;
    }
    return pll;
}

//========================================================================================================================================================================================================================
// renormalisation constant for spherical functions, m != 0, sqrt(2) is included as a factor
//========================================================================================================================================================================================================================
double K(int l, int m)
{
    double k = ((2.0 * l + 1.0) * factorial(l - m)) / (constants::two_pi * factorial(l + m));
    return glm::sqrt(k);
}

double K(int l)             /* Special case m = 0 */
{
    double k = (l + 0.5) / constants::two_pi;
    return glm::sqrt(k);
}

//========================================================================================================================================================================================================================
// return a point sample of a Spherical Harmonic basis function
// l is the band, non-negative integer < L
// m is integer satisfying -l <= m <= l
// theta in the range [0..pi]
// phi in the range [0..two_pi]
//========================================================================================================================================================================================================================
double sh(int l, int m, double theta, double phi)
{
    double z = glm::cos(theta);

    if (m > 0)
        return K(l,  m) * glm::cos( m * phi) * P(l,  m, z);

    if (m < 0)
        return K(l, -m) * glm::sin(-m * phi) * P(l, -m, z);

    return K(l) * P(l, 0, z);
}



void main()
{
    sh_sampler_t harmonic_sampler(64, 5);

}
