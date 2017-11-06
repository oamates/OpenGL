
    #include <fftw3.h>
    #include <complex>
    #include <vector>
    
    void reconstruct_height_map(const float *normal, float *dx, float *dy, int width, int height, float *result)
    {
        typedef std::complex<float> C;
        fftwf_plan plan;
    
        std::vector<float> nx(width*height), ny(width*height);
        for(int y = 0, i = 0; y < height; ++y)
        for(int x = 0; x < width; ++x, ++i, normal += 3)
            nx[i] = normal[0]/normal[2], ny[i] = normal[1]/normal[2];
    
        const int half_width = width/2 + 1;
        std::vector<C> Nx(half_width*height), Ny(half_width*height);
        std::vector<C> Dx(half_width*height), Dy(half_width*height);
    
        plan = fftwf_plan_dft_r2c_2d(height, width, &nx[0], (fftwf_complex*)&Nx[0], FFTW_ESTIMATE);
        fftwf_execute_dft_r2c(plan, &nx[0], (fftwf_complex*)&Nx[0]);
        fftwf_execute_dft_r2c(plan, &ny[0], (fftwf_complex*)&Ny[0]);
        fftwf_execute_dft_r2c(plan, &dx[0], (fftwf_complex*)&Dx[0]);
        fftwf_execute_dft_r2c(plan, &dy[0], (fftwf_complex*)&Dy[0]);
        fftwf_destroy_plan(plan);
    
        std::vector<C> F(half_width*height);
        for(int y = 0, i = 0; y < height; ++y)
        for(int x = 0; x < half_width; ++x, ++i)
        {
            float denom = width * height * (norm(Dx[i]) + norm(Dy[i]));
            F[i] = denom > 0 ? - (Dx[i] * Nx[i] + Dy[i] * Ny[i]) / denom : 0;
        }
    
        plan = fftwf_plan_dft_c2r_2d(height, width, (fftwf_complex*)&F[0], &result[0], FFTW_ESTIMATE);
        fftwf_execute(plan);
        fftwf_destroy_plan(plan);
    }
    
    void reconstruct_height_map1(const float *normal, int width, int height, float *result)
    {
        std::vector<float> dx(width*height), dy(width*height);
        dx[0] = 1, dx[1] = -1;
        dy[0] = 1, dy[width] = -1;
        reconstruct_height_map(normal, &dx[0], &dy[0], width, height, result);
    }
    
    void reconstruct_height_map2(const float *normal, int width, int height, float *result)
    {
        std::vector<float> dx(width*height), dy(width*height);
        dx[width-1] = 1, dx[1] = -1;
        dy[width*(height-1)] = 1, dy[width] = -1;
        reconstruct_height_map(normal, &dx[0], &dy[0], width, height, result);
    }