@call gcc -mavx2 -mfma -S -O3  -fabi-version=6 -std=c++11 avx2.cpp 
@call gcc -mavx2 -mfma -S -O3  -fabi-version=6 -std=c++11 sse41.cpp 
@call gcc -mavx2 -mfma -S -O3  -fabi-version=6 -std=c++11 sse2.cpp
@call gcc -mavx2 -mfma -S -O3  -fabi-version=6 -std=c++11 value.cpp 
@call gcc -mavx2 -mfma -S -O3  -fabi-version=6 -std=c++11 gradient.cpp 
@call gcc -mavx2 -mfma -S -O3  -fabi-version=6 -std=c++11 simplex.cpp  