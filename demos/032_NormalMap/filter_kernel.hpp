#ifndef _filter_kernel_included_18563487516089746501873465081734658017365801374
#define _filter_kernel_included_18563487516089746501873465081734658017365801374

struct kernel_element_t
{
    int x, y;
    float w;
};

//=======================================================================================================================================================================================================================
// symmetric difference kernel
//=======================================================================================================================================================================================================================
static const int FILTER_SYM_DIFF_KERNEL_SIZE = 2;
static kernel_element_t FILTER_SYM_DIFF_KERNEL_DU[] = 
{
    {-1,  0, -0.5f},
    { 1,  0,  0.5f}
};

static kernel_element_t FILTER_SYM_DIFF_KERNEL_DV[] = 
{
    { 0,  1,  0.5f},
    { 0, -1, -0.5f}
};

//=======================================================================================================================================================================================================================
// 3x3 Sobel filter
//=======================================================================================================================================================================================================================
static const int FILTER_SOBEL_3x3_KERNEL_SIZE = 6;
static kernel_element_t FILTER_SOBEL_3x3_KERNEL_DU[] = 
{
    {-1,  1, -1.0f},
    {-1,  0, -2.0f},
    {-1, -1, -1.0f},
    { 1,  1,  1.0f},
    { 1,  0,  2.0f},
    { 1, -1,  1.0f}
};

static kernel_element_t FILTER_SOBEL_3x3_KERNEL_DV[] =
{
    {-1,  1,  1.0f},
    { 0,  1,  2.0f},
    { 1,  1,  1.0f},
    {-1, -1, -1.0f},
    { 0, -1, -2.0f},
    { 1, -1, -1.0f},
};

//=======================================================================================================================================================================================================================
// 5x5 Sobel filter
//=======================================================================================================================================================================================================================
static const int FILTER_SOBEL_5x5_KERNEL_SIZE = 20;
static kernel_element_t FILTER_SOBEL_5x5_KERNEL_DU[] = 
{
    {-2,  2,  -1.0f},
    {-2,  1,  -4.0f},
    {-2,  0,  -6.0f},
    {-2, -1,  -4.0f},
    {-2, -2,  -1.0f},
    {-1,  2,  -2.0f},
    {-1,  1,  -8.0f},
    {-1,  0, -12.0f},
    {-1, -1,  -8.0f},
    {-1, -2,  -2.0f},
    { 1,  2,   2.0f},
    { 1,  1,   8.0f},
    { 1,  0,  12.0f},
    { 1, -1,   8.0f},
    { 1, -2,   2.0f},
    { 2,  2,   1.0f},
    { 2,  1,   4.0f},
    { 2,  0,   6.0f},
    { 2, -1,   4.0f},
    { 2, -2,   1.0f} 
};

static kernel_element_t FILTER_SOBEL_5x5_KERNEL_DV[] =
{
    {-2,  2,   1.0f},
    {-1,  2,   4.0f},
    { 0,  2,   6.0f},
    { 1,  2,   4.0f},
    { 2,  2,   1.0f},
    {-2,  1,   2.0f},
    {-1,  1,   8.0f},
    { 0,  1,  12.0f},
    { 1,  1,   8.0f},
    { 2,  1,   2.0f},
    {-2, -1,  -2.0f},
    {-1, -1,  -8.0f},
    { 0, -1, -12.0f},
    { 1, -1,  -8.0f},
    { 2, -1,  -2.0f},
    {-2, -2,  -1.0f},
    {-1, -2,  -4.0f},
    { 0, -2,  -6.0f},
    { 1, -2,  -4.0f},
    { 2, -2,  -1.0f}
};

//=======================================================================================================================================================================================================================
// 3x3 Prewitt partial derivative kernel
//=======================================================================================================================================================================================================================
static const int FILTER_PREWITT_3x3_KERNEL_SIZE = 6;
static kernel_element_t FILTER_PREWITT_3x3_KERNEL_DU[] = 
{
    {-1,  1, -1.0f},
    {-1,  0, -1.0f},
    {-1, -1, -1.0f},
    { 1,  1,  1.0f},
    { 1,  0,  1.0f},
    { 1, -1,  1.0f}
};

static kernel_element_t FILTER_PREWITT_3x3_KERNEL_DV[] =
{
    {-1,  1,  1.0f},
    { 0,  1,  1.0f},
    { 1,  1,  1.0f},
    {-1, -1, -1.0f},
    { 0, -1, -1.0f},
    { 1, -1, -1.0f}
};

//=======================================================================================================================================================================================================================
// 5x5 Prewitt partial derivative kernel
//=======================================================================================================================================================================================================================
static const int FILTER_PREWITT_5x5_KERNEL_SIZE = 20;
static kernel_element_t FILTER_PREWITT_5x5_KERNEL_DU[] = 
{
    {-2,  2, -1.0f},
    {-2,  1, -1.0f},
    {-2,  0, -1.0f},
    {-2, -1, -1.0f},
    {-2, -2, -1.0f},
    {-1,  2, -2.0f},
    {-1,  1, -2.0f},
    {-1,  0, -2.0f},
    {-1, -1, -2.0f},
    {-1, -2, -2.0f},
    { 1,  2,  2.0f},
    { 1,  1,  2.0f},
    { 1,  0,  2.0f},
    { 1, -1,  2.0f},
    { 1, -2,  2.0f},
    { 2,  2,  1.0f},
    { 2,  1,  1.0f},
    { 2,  0,  1.0f},
    { 2, -1,  1.0f},
    { 2, -2,  1.0f}
};

static kernel_element_t FILTER_PREWITT_5x5_KERNEL_DV[] = 
{
    {-2,  2,  1.0f},
    {-1,  2,  1.0f},
    { 0,  2,  1.0f},
    { 1,  2,  1.0f},
    { 2,  2,  1.0f},
    {-2,  1,  2.0f},
    {-1,  1,  2.0f},
    { 0,  1,  2.0f},
    { 1,  1,  2.0f},
    { 2,  1,  2.0f},
    {-2, -1, -2.0f},
    {-1, -1, -2.0f},
    { 0, -1, -2.0f},
    { 1, -1, -2.0f},
    { 2, -1, -2.0f},
    {-2, -2, -1.0f},
    {-1, -2, -1.0f},
    { 0, -2, -1.0f},
    { 1, -2, -1.0f},
    { 2, -2, -1.0f}
};

//=======================================================================================================================================================================================================================
// 3x3 partial derivative kernel
//=======================================================================================================================================================================================================================
static const float filter_3x3_weight = 1.0f / 6.0f;
static const int FILTER_3x3_KERNEL_SIZE = 6;
static kernel_element_t FILTER_3x3_KERNEL_DU[] = 
{
    {-1,  1, -filter_3x3_weight},
    {-1,  0, -filter_3x3_weight},
    {-1, -1, -filter_3x3_weight},
    { 1,  1,  filter_3x3_weight},
    { 1,  0,  filter_3x3_weight},
    { 1, -1,  filter_3x3_weight}
};

static kernel_element_t FILTER_3x3_KERNEL_DV[] = 
{
    {-1,  1, -filter_3x3_weight},
    {-1,  0, -filter_3x3_weight},
    {-1, -1, -filter_3x3_weight},
    { 1,  1,  filter_3x3_weight},
    { 1,  0,  filter_3x3_weight},
    { 1, -1,  filter_3x3_weight}
};

//=======================================================================================================================================================================================================================
// 5x5 partial derivative kernel
//=======================================================================================================================================================================================================================
static const float filter_5x5_weight = 1.0f / 6.0f;
static const int FILTER_3x3_KERNEL_SIZE = 6;
static kernel_element_t FILTER_3x3_KERNEL_DU[] = 
{
    {-2, -2, -filter_5x5_weight},
    {-2, -1, -filter_5x5_weight},
    {-2,  0, -filter_5x5_weight},
    {-2,  1, -filter_5x5_weight},
    {-2,  2, -filter_5x5_weight},

    {-1,  1, -filter_3x3_weight},
    {-1,  0, -filter_3x3_weight},
    {-1, -1, -filter_3x3_weight},

    { 1,  1,  filter_3x3_weight},
    { 1,  0,  filter_3x3_weight},
    { 1, -1,  filter_3x3_weight}
};

static kernel_element_t FILTER_3x3_KERNEL_DV[] = 
{
    {-1,  1, -filter_3x3_weight},
    {-1,  0, -filter_3x3_weight},
    {-1, -1, -filter_3x3_weight},
    { 1,  1,  filter_3x3_weight},
    { 1,  0,  filter_3x3_weight},
    { 1, -1,  filter_3x3_weight}
};

            int n;
            float usum = 0, vsum = 0;
            float wt22 = 1.0f / 16.0f;
            float wt12 = 1.0f / 10.0f;
            float wt02 = 1.0f / 8.0f;
            float wt11 = 1.0f / 2.8f;
            kernel_size = 20;
            kernel_du = (kernel_element_t*)malloc(20 * sizeof(kernel_element_t));
            kernel_dv = (kernel_element_t*)malloc(20 * sizeof(kernel_element_t));
         
            kernel_du[0 ] = {-2,  2, -wt22};
            kernel_du[1 ] = {-1,  2, -wt12};
            kernel_du[2 ] = { 1,  2,  wt12};
            kernel_du[3 ] = { 2,  2,  wt22};
            kernel_du[4 ] = {-2,  1, -wt12};
            kernel_du[5 ] = {-1,  1, -wt11};
            kernel_du[6 ] = { 1,  1,  wt11};
            kernel_du[7 ] = { 2,  1,  wt12};
            kernel_du[8 ] = {-2,  0, -wt02};
            kernel_du[9 ] = {-1,  0, -0.5f};
            kernel_du[10] = { 1,  0,  0.5f};
            kernel_du[11] = { 2,  0,  wt02};
            kernel_du[12] = {-2, -1, -wt12};
            kernel_du[13] = {-1, -1, -wt11};
            kernel_du[14] = { 1, -1,  wt11};
            kernel_du[15] = { 2, -1,  wt12};
            kernel_du[16] = {-2, -2, -wt22};
            kernel_du[17] = {-1, -2, -wt12};
            kernel_du[18] = { 1, -2,  wt12};
            kernel_du[19] = { 2, -2,  wt22};
         
            kernel_dv[0 ] = {-2,  2,  wt22};
            kernel_dv[1 ] = {-1,  2,  wt12};
            kernel_dv[2 ] = { 0,  2, 0.25f};
            kernel_dv[3 ] = { 1,  2,  wt12};
            kernel_dv[4 ] = { 2,  2,  wt22};
            kernel_dv[5 ] = {-2,  1,  wt12};
            kernel_dv[6 ] = {-1,  1,  wt11};
            kernel_dv[7 ] = { 0,  1,  0.5f};
            kernel_dv[8 ] = { 1,  1,  wt11};
            kernel_dv[9 ] = { 2,  1,  wt22};
            kernel_dv[10] = {-2, -1, -wt22};
            kernel_dv[11] = {-1, -1, -wt11};
            kernel_dv[12] = { 0, -1, -0.5f};
            kernel_dv[13] = { 1, -1, -wt11};
            kernel_dv[14] = { 2, -1, -wt12};
            kernel_dv[15] = {-2, -2, -wt22};
            kernel_dv[16] = {-1, -2, -wt12};
            kernel_dv[17] = { 0, -2,-0.25f};
            kernel_dv[18] = { 1, -2, -wt12};
            kernel_dv[19] = { 2, -2, -wt22};

            for(n = 0; n < 20; ++n)
            {
               usum += fabsf(kernel_du[n].w);
               vsum += fabsf(kernel_dv[n].w);
            }
            for(n = 0; n < 20; ++n)
            {
               kernel_du[n].w /= usum;
               kernel_dv[n].w /= vsum;
            }
         
            break;



//=======================================================================================================================================================================================================================
// 7x7 partial derivative kernel
//=======================================================================================================================================================================================================================
static const float filter_7x7_weight = 1.0f / 138.0f;
static const int FILTER_7x7_KERNEL_SIZE = 42;
static kernel_element_t FILTER_7x7_KERNEL_DU[] =
{
    {-3, -3, -1.0f * filter_7x7_weight},
    {-3, -2, -2.0f * filter_7x7_weight},
    {-3, -1, -3.0f * filter_7x7_weight},
    {-3,  0, -4.0f * filter_7x7_weight},
    {-3,  1, -3.0f * filter_7x7_weight},
    {-3,  2, -2.0f * filter_7x7_weight},
    {-3,  3, -1.0f * filter_7x7_weight},
    {-2, -3, -2.0f * filter_7x7_weight},
    {-2, -2, -3.0f * filter_7x7_weight},
    {-2, -1, -4.0f * filter_7x7_weight},
    {-2,  0, -5.0f * filter_7x7_weight},
    {-2,  1, -4.0f * filter_7x7_weight},
    {-2,  2, -3.0f * filter_7x7_weight},
    {-2,  3, -2.0f * filter_7x7_weight},
    {-1, -3, -3.0f * filter_7x7_weight},
    {-1, -2, -4.0f * filter_7x7_weight},
    {-1, -1, -5.0f * filter_7x7_weight},
    {-1,  0, -6.0f * filter_7x7_weight},
    {-1,  1, -5.0f * filter_7x7_weight},
    {-1,  2, -4.0f * filter_7x7_weight},
    {-1,  3, -3.0f * filter_7x7_weight},
    { 1, -3,  3.0f * filter_7x7_weight},
    { 1, -2,  4.0f * filter_7x7_weight},
    { 1, -1,  5.0f * filter_7x7_weight},
    { 1,  0,  6.0f * filter_7x7_weight},
    { 1,  1,  5.0f * filter_7x7_weight},
    { 1,  2,  4.0f * filter_7x7_weight},
    { 1,  3,  3.0f * filter_7x7_weight},
    { 2, -3,  2.0f * filter_7x7_weight},
    { 2, -2,  3.0f * filter_7x7_weight},
    { 2, -1,  4.0f * filter_7x7_weight},
    { 2,  0,  5.0f * filter_7x7_weight},
    { 2,  1,  4.0f * filter_7x7_weight},
    { 2,  2,  3.0f * filter_7x7_weight},
    { 2,  3,  2.0f * filter_7x7_weight},
    { 3, -3,  1.0f * filter_7x7_weight},
    { 3, -2,  2.0f * filter_7x7_weight},
    { 3, -1,  3.0f * filter_7x7_weight},
    { 3,  0,  4.0f * filter_7x7_weight},
    { 3,  1,  3.0f * filter_7x7_weight},
    { 3,  2,  2.0f * filter_7x7_weight},
    { 3,  3,  1.0f * filter_7x7_weight}
};

static kernel_element_t FILTER_7x7_KERNEL_DV[] = 
{
    {-3, -3, -1.0f * filter_7x7_weight},
    {-2, -3, -2.0f * filter_7x7_weight},
    {-1, -3, -3.0f * filter_7x7_weight},
    { 0, -3, -4.0f * filter_7x7_weight},
    { 1, -3, -3.0f * filter_7x7_weight},
    { 2, -3, -2.0f * filter_7x7_weight},
    { 3, -3, -1.0f * filter_7x7_weight},
    {-3, -2, -2.0f * filter_7x7_weight},
    {-2, -2, -3.0f * filter_7x7_weight},
    {-1, -2, -4.0f * filter_7x7_weight},
    { 0, -2, -5.0f * filter_7x7_weight},
    { 1, -2, -4.0f * filter_7x7_weight},
    { 2, -2, -3.0f * filter_7x7_weight},
    { 3, -2, -2.0f * filter_7x7_weight},
    {-3, -1, -3.0f * filter_7x7_weight},
    {-2, -1, -4.0f * filter_7x7_weight},
    {-1, -1, -5.0f * filter_7x7_weight},
    { 0, -1, -6.0f * filter_7x7_weight},
    { 1, -1, -5.0f * filter_7x7_weight},
    { 2, -1, -4.0f * filter_7x7_weight},
    { 3, -1, -3.0f * filter_7x7_weight},
    {-3,  1, -3.0f * filter_7x7_weight},
    {-2,  1, -4.0f * filter_7x7_weight},
    {-1,  1, -5.0f * filter_7x7_weight},
    { 0,  1, -6.0f * filter_7x7_weight},
    { 1,  1, -5.0f * filter_7x7_weight},
    { 2,  1, -4.0f * filter_7x7_weight},
    { 3,  1, -3.0f * filter_7x7_weight},
    {-3,  2, -2.0f * filter_7x7_weight},
    {-2,  2, -3.0f * filter_7x7_weight},
    {-1,  2, -4.0f * filter_7x7_weight},
    { 0,  2, -5.0f * filter_7x7_weight},
    { 1,  2, -4.0f * filter_7x7_weight},
    { 2,  2, -3.0f * filter_7x7_weight},
    { 3,  2, -2.0f * filter_7x7_weight},
    {-3,  3, -1.0f * filter_7x7_weight},
    {-2,  3, -2.0f * filter_7x7_weight},
    {-1,  3, -3.0f * filter_7x7_weight},
    { 0,  3, -4.0f * filter_7x7_weight},
    { 1,  3, -3.0f * filter_7x7_weight},
    { 2,  3, -2.0f * filter_7x7_weight},
    { 3,  3, -1.0f * filter_7x7_weight}
};

//=======================================================================================================================================================================================================================
// 9x9 partial derivative kernel
//=======================================================================================================================================================================================================================
static const float filter_9x9_weight = 1.0f / 308.0f;
static const int FILTER_9x9_KERNEL_SIZE = 72;
static kernel_element_t FILTER_9x9_KERNEL_DU[] =
{
    {-4, -4, -1.0f * filter_9x9_weight},
    {-4, -3, -2.0f * filter_9x9_weight},
    {-4, -2, -3.0f * filter_9x9_weight},
    {-4, -1, -4.0f * filter_9x9_weight},
    {-4,  0, -5.0f * filter_9x9_weight},
    {-4,  1, -4.0f * filter_9x9_weight},
    {-4,  2, -3.0f * filter_9x9_weight},
    {-4,  3, -2.0f * filter_9x9_weight},
    {-4,  4, -1.0f * filter_9x9_weight},
    {-3, -4, -2.0f * filter_9x9_weight},
    {-3, -3, -3.0f * filter_9x9_weight},
    {-3, -2, -4.0f * filter_9x9_weight},
    {-3, -1, -5.0f * filter_9x9_weight},
    {-3,  0, -6.0f * filter_9x9_weight},
    {-3,  1, -5.0f * filter_9x9_weight},
    {-3,  2, -4.0f * filter_9x9_weight},
    {-3,  3, -3.0f * filter_9x9_weight},
    {-3,  4, -2.0f * filter_9x9_weight},
    {-2, -4, -3.0f * filter_9x9_weight},
    {-2, -3, -4.0f * filter_9x9_weight},
    {-2, -2, -5.0f * filter_9x9_weight},
    {-2, -1, -6.0f * filter_9x9_weight},
    {-2,  0, -7.0f * filter_9x9_weight},
    {-2,  1, -6.0f * filter_9x9_weight},
    {-2,  2, -5.0f * filter_9x9_weight},
    {-2,  3, -4.0f * filter_9x9_weight},
    {-2,  4, -3.0f * filter_9x9_weight},
    {-1, -4, -4.0f * filter_9x9_weight},
    {-1, -3, -5.0f * filter_9x9_weight},
    {-1, -2, -6.0f * filter_9x9_weight},
    {-1, -1, -7.0f * filter_9x9_weight},
    {-1,  0, -8.0f * filter_9x9_weight},
    {-1,  1, -7.0f * filter_9x9_weight},
    {-1,  2, -6.0f * filter_9x9_weight},
    {-1,  3, -5.0f * filter_9x9_weight},
    {-1,  4, -4.0f * filter_9x9_weight},
    { 1, -4,  4.0f * filter_9x9_weight},
    { 1, -3,  5.0f * filter_9x9_weight},
    { 1, -2,  6.0f * filter_9x9_weight},
    { 1, -1,  7.0f * filter_9x9_weight},
    { 1,  0,  8.0f * filter_9x9_weight},
    { 1,  1,  7.0f * filter_9x9_weight},
    { 1,  2,  6.0f * filter_9x9_weight},
    { 1,  3,  5.0f * filter_9x9_weight},
    { 1,  4,  4.0f * filter_9x9_weight},
    { 2, -4,  3.0f * filter_9x9_weight},
    { 2, -3,  4.0f * filter_9x9_weight},
    { 2, -2,  5.0f * filter_9x9_weight},
    { 2, -1,  6.0f * filter_9x9_weight},
    { 2,  0,  7.0f * filter_9x9_weight},
    { 2,  1,  6.0f * filter_9x9_weight},
    { 2,  2,  5.0f * filter_9x9_weight},
    { 2,  3,  4.0f * filter_9x9_weight},
    { 2,  4,  3.0f * filter_9x9_weight},
    { 3, -4,  2.0f * filter_9x9_weight},
    { 3, -3,  3.0f * filter_9x9_weight},
    { 3, -2,  4.0f * filter_9x9_weight},
    { 3, -1,  5.0f * filter_9x9_weight},
    { 3,  0,  6.0f * filter_9x9_weight},
    { 3,  1,  5.0f * filter_9x9_weight},
    { 3,  2,  4.0f * filter_9x9_weight},
    { 3,  3,  3.0f * filter_9x9_weight},
    { 3,  4,  2.0f * filter_9x9_weight},
    { 4, -4,  1.0f * filter_9x9_weight},
    { 4, -3,  2.0f * filter_9x9_weight},
    { 4, -2,  3.0f * filter_9x9_weight},
    { 4, -1,  4.0f * filter_9x9_weight},
    { 4,  0,  5.0f * filter_9x9_weight},
    { 4,  1,  4.0f * filter_9x9_weight},
    { 4,  2,  3.0f * filter_9x9_weight},
    { 4,  3,  2.0f * filter_9x9_weight},
    { 4,  4,  1.0f * filter_9x9_weight}
};

static kernel_element_t FILTER_9x9_KERNEL_DV[] = 
{
    {-4, -4, -1.0f * filter_9x9_weight},
    {-3, -4, -2.0f * filter_9x9_weight},
    {-2, -4, -3.0f * filter_9x9_weight},
    {-1, -4, -4.0f * filter_9x9_weight},
    { 0, -4, -5.0f * filter_9x9_weight},
    { 1, -4, -4.0f * filter_9x9_weight},
    { 2, -4, -3.0f * filter_9x9_weight},
    { 3, -4, -2.0f * filter_9x9_weight},
    { 4, -4, -1.0f * filter_9x9_weight},
    {-4, -3, -2.0f * filter_9x9_weight},
    {-3, -3, -3.0f * filter_9x9_weight},
    {-2, -3, -4.0f * filter_9x9_weight},
    {-1, -3, -5.0f * filter_9x9_weight},
    { 0, -3, -6.0f * filter_9x9_weight},
    { 1, -3, -5.0f * filter_9x9_weight},
    { 2, -3, -4.0f * filter_9x9_weight},
    { 3, -3, -3.0f * filter_9x9_weight},
    { 4, -3, -2.0f * filter_9x9_weight},
    {-4, -2, -3.0f * filter_9x9_weight},
    {-3, -2, -4.0f * filter_9x9_weight},
    {-2, -2, -5.0f * filter_9x9_weight},
    {-1, -2, -6.0f * filter_9x9_weight},
    { 0, -2, -7.0f * filter_9x9_weight},
    { 1, -2, -6.0f * filter_9x9_weight},
    { 2, -2, -5.0f * filter_9x9_weight},
    { 3, -2, -4.0f * filter_9x9_weight},
    { 4, -2, -3.0f * filter_9x9_weight},
    {-4, -1, -4.0f * filter_9x9_weight},
    {-3, -1, -5.0f * filter_9x9_weight},
    {-2, -1, -6.0f * filter_9x9_weight},
    {-1, -1, -7.0f * filter_9x9_weight},
    { 0, -1, -8.0f * filter_9x9_weight},
    { 1, -1, -7.0f * filter_9x9_weight},
    { 2, -1, -6.0f * filter_9x9_weight},
    { 3, -1, -5.0f * filter_9x9_weight},
    { 4, -1, -4.0f * filter_9x9_weight},
    {-4,  1,  4.0f * filter_9x9_weight},
    {-3,  1,  5.0f * filter_9x9_weight},
    {-2,  1,  6.0f * filter_9x9_weight},
    {-1,  1,  7.0f * filter_9x9_weight},
    { 0,  1,  8.0f * filter_9x9_weight},
    { 1,  1,  7.0f * filter_9x9_weight},
    { 2,  1,  6.0f * filter_9x9_weight},
    { 3,  1,  5.0f * filter_9x9_weight},
    { 4,  1,  4.0f * filter_9x9_weight},
    {-4,  2,  3.0f * filter_9x9_weight},
    {-3,  2,  4.0f * filter_9x9_weight},
    {-2,  2,  5.0f * filter_9x9_weight},
    {-1,  2,  6.0f * filter_9x9_weight},
    { 0,  2,  7.0f * filter_9x9_weight},
    { 1,  2,  6.0f * filter_9x9_weight},
    { 2,  2,  5.0f * filter_9x9_weight},
    { 3,  2,  4.0f * filter_9x9_weight},
    { 4,  2,  3.0f * filter_9x9_weight},
    {-4,  3,  2.0f * filter_9x9_weight},
    {-3,  3,  3.0f * filter_9x9_weight},
    {-2,  3,  4.0f * filter_9x9_weight},
    {-1,  3,  5.0f * filter_9x9_weight},
    { 0,  3,  6.0f * filter_9x9_weight},
    { 1,  3,  5.0f * filter_9x9_weight},
    { 2,  3,  4.0f * filter_9x9_weight},
    { 3,  3,  3.0f * filter_9x9_weight},
    { 4,  3,  2.0f * filter_9x9_weight},
    {-4,  4,  1.0f * filter_9x9_weight},
    {-3,  4,  2.0f * filter_9x9_weight},
    {-2,  4,  3.0f * filter_9x9_weight},
    {-1,  4,  4.0f * filter_9x9_weight},
    { 0,  4,  5.0f * filter_9x9_weight},
    { 1,  4,  4.0f * filter_9x9_weight},
    { 2,  4,  3.0f * filter_9x9_weight},
    { 3,  4,  2.0f * filter_9x9_weight},
    { 4,  4,  1.0f * filter_9x9_weight}
};

#endif // _filter_kernel_included_18563487516089746501873465081734658017365801374
