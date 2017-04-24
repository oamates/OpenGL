#version 430 core

//========================================================================================================================================================================================================================
// The shader does n x n FFT algorithm
//========================================================================================================================================================================================================================
const uint n = 1024;
const uint m = 10;          // = log2(n)


//========================================================================================================================================================================================================================
template<typename real_t, bool inverse> void transform1d(complex_t<real_t>* z, unsigned int m, unsigned int stride)
{
    unsigned int n = 1 << m;
    unsigned int i2 = n >> 1;
    unsigned int j = 0;
    for (unsigned int i = 0; i < n - 1; i++) 
    {
        if (i < j)
        {
            complex_t<real_t> t = z[stride * i];
            z[stride * i] = z[stride * j];
            z[stride * j] = t;
        }
        unsigned int k = i2;
        while (k <= j) 
        {
            j -= k;
            k >>= 1;
        }
        j += k;
    }

    complex_t<real_t> c(-1.0, 0.0); 
    unsigned int l2 = 1;
    for (unsigned int l = 0; l < m; l++)
    {
        unsigned int l1 = l2;
        l2 += l2;
        complex_t<real_t> u(1.0, 0.0);
        for (j = 0; j < l1; j++)
        { 
            for (unsigned int i = j; i < n; i += l2)
            {
                unsigned int i1 = i + l1;
                complex_t<real_t> t = u * z[stride * i1];
                z[stride * i1] = z[stride * i] - t;
                z[stride * i] += t;
            }
            u *= c;
        }
        c.im = -sqrt((1.0 - c.re) / 2.0);
        if (inverse)
            c.im = -c.im;
        c.re =  sqrt((1.0 + c.re) / 2.0);
    }
}


//========================================================================================================================================================================================================================
// Input buffer :: the input data is a complex (packed as vec2) N x N matrix
// No bit reversing step is performed, therefore the first butterfly step should be different from the
// following and essentially perform both bit reversing and butterfly.
// It make sence to separate it in any case as for the first butterfly step roots of unity 
// are so simple that no complex multiplication is required at all
//========================================================================================================================================================================================================================
layout(std140, binding = 0) buffer input
{
    vec2 z[ ];
};
 
layout(std140, binding = 1) buffer output
{
    vec2 Z[ ];
};

//========================================================================================================================================================================================================================
// Every invocation will work with 4 indices simultaneously producing FFT4-butterfly on each step 
// The shader should be invoked with glDispatchCompute(1, N, 1)
// This way every workgroup will be working with a single row of an SSBO 2D input array
//   and synchronization will be necessary only inside for invocations in a given workgroup 
//========================================================================================================================================================================================================================
layout(local_size_x = N / 4, local_size_y = 1, local_size_z = 1) in;

void main()
{
    //====================================================================================================================================================================================================================
    // We assume the input data is in bit reversed order and 
    // compute will be initiated with glDispatchCompute(N / 8, N, 1) call
    //====================================================================================================================================================================================================================
    uint iX = gl_GlobalInvocationID.x;
    uint iY = gl_GlobalInvocationID.y;

    //====================================================================================================================================================================================================================
    // First, let us perform row FFT4 transform ::
    //   Z0 = ((z0 + z4) + 1(z2 + z6)) +  1((z1 + z5) + 1(z3 + z7));
    //   Z1 = ((z0 - z4) + i(z2 - z6)) +  w((z1 - z5) + i(z3 - z7));
    //   Z2 = ((z0 + z4) - 1(z2 + z6)) +  i((z1 + z5) - 1(z3 + z7));
    //   Z3 = ((z0 - z4) - i(z2 - z6)) + iw((z1 - z5) - i(z3 - z7));
    //   Z4 = ((z0 + z4) + 1(z2 + z6)) -  1((z1 + z5) + 1(z3 + z7));
    //   Z5 = ((z0 - z4) + i(z2 - z6)) -  w((z1 - z5) + i(z3 - z7));
    //   Z6 = ((z0 + z4) - 1(z2 + z6)) -  i((z1 + z5) - 1(z3 + z7));
    //   Z7 = ((z0 - z4) - i(z2 - z6)) - iw((z1 - z5) - i(z3 - z7));
    //====================================================================================================================================================================================================================
}


//========================================================================================================================================================================================================================
// Every invocation will work with 8 indices simultaneously producing FFT8-butterfly on each step 
// The shader should be invoked with glDispatchCompute(1, N, 1)
// This way every workgroup will be working with a single row of an SSBO 2D input array
//   and synchronization will be necessary only inside for invocations in a given workgroup 
//========================================================================================================================================================================================================================
//layout(local_size_x = N / 8, local_size_y = 1, local_size_z = 1) in;





void main()
{
    //====================================================================================================================================================================================================================
    // We assume the input data is in bit reversed order and 
    // compute will be initiated with glDispatchCompute(N / 8, N, 1) call
    //====================================================================================================================================================================================================================
    uint iX = gl_GlobalInvocationID.x;
    uint iY = gl_GlobalInvocationID.y;

    //====================================================================================================================================================================================================================
    // First, let us perform row FFT8 transform ::
    //   Z0 = ((z0 + z4) + 1(z2 + z6)) +  1((z1 + z5) + 1(z3 + z7));
    //   Z1 = ((z0 - z4) + i(z2 - z6)) +  w((z1 - z5) + i(z3 - z7));
    //   Z2 = ((z0 + z4) - 1(z2 + z6)) +  i((z1 + z5) - 1(z3 + z7));
    //   Z3 = ((z0 - z4) - i(z2 - z6)) + iw((z1 - z5) - i(z3 - z7));
    //   Z4 = ((z0 + z4) + 1(z2 + z6)) -  1((z1 + z5) + 1(z3 + z7));
    //   Z5 = ((z0 - z4) + i(z2 - z6)) -  w((z1 - z5) + i(z3 - z7));
    //   Z6 = ((z0 + z4) - 1(z2 + z6)) -  i((z1 + z5) - 1(z3 + z7));
    //   Z7 = ((z0 - z4) - i(z2 - z6)) - iw((z1 - z5) - i(z3 - z7));
    //====================================================================================================================================================================================================================

    uint base_index = N * iY + 8 * iX;

    vec2 z01p = z[base_index + 0] + z[base_index + 1];
    vec2 z01m = z[base_index + 0] - z[base_index + 1];
    vec2 z23p = z[base_index + 2] + z[base_index + 3];
    vec2 z23m = z[base_index + 2] - z[base_index + 3];
    vec2 z45p = z[base_index + 4] + z[base_index + 5];
    vec2 z45m = z[base_index + 4] - z[base_index + 5];
    vec2 z67p = z[base_index + 6] + z[base_index + 7];
    vec2 z67m = z[base_index + 6] - z[base_index + 7];

    vec2 iz23m = vec2(iz23m.y, -iz23m.x);
    vec2 iz67m = vec2(iz67m.y, -iz67m.x);

    vec2 z0123p1p = z01p +  z23p;
    vec2 z0123p1m = z01p -  z23p;
    vec2 z0123mip = z01m + iz23m;
    vec2 z0123mim = z01m - iz23m;
    vec2 z4567p1p = z45p +  z67p;
    vec2 z4567p1m = z45p -  z67p;
    vec2 z4567mip = z45m + iz67m;
    vec2 z4567mim = z45m - iz67m;

    vec2  iz4567p1m = vec2(iz4567p1m.y, -iz4567p1m.x);
    vec2  wz4567mip = inv_sqrt2 * vec2( z4567mip.x - z4567mip.y, z4567mip.x + z4567mip.y);
    vec2 iwz4567mim = inv_sqrt2 * vec2(-z4567mim.x - z4567mim.y, z4567mim.x - z4567mim.y);

    Z[base_index + 0] = z0123p1p +  1z4567p1p;
    Z[base_index + 1] = z0123mip +  wz4567mip;
    Z[base_index + 2] = z0123p1m +  iz4567p1m;
    Z[base_index + 3] = z0123mim + iwz4567mim;
    Z[base_index + 4] = z0123p1p -  1z4567p1p;
    Z[base_index + 5] = z0123mip -  wz4567mip;
    Z[base_index + 6] = z0123p1m -  iz4567p1m;
    Z[base_index + 7] = z0123mim - iwz4567mim;

    barrier();

    complex_t<real_t> c(-1.0, 0.0); 
    uint l2 = 8;
    for (uint l = 3; l < m; l++)
    {
        

        
        uint l1 = l2;
        l2 += l2;
        complex_t<real_t> u(1.0, 0.0);
        for (uint j = 0; j < l1; j++)
        { 
            for (uint i = j; i < n; i += l2)
            {
                uint i1 = i + l1;
                complex_t<real_t> t = u * z[i1];
                z[i1] = z[i] - t;
                z[i] += t;
            }
            u *= c;
        }
        c.im = -sqrt((1.0 - c.re) / 2.0);
        if (inverse)
            c.im = -c.im;
        c.re =  sqrt((1.0 + c.re) / 2.0);
    }

}

q q + 1
q + 2 q + 3


q q + 4 
q + 8 q + 12 


