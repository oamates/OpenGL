#version 430 core

// complex multiplication
vec2 cmul(vec2 z, vec2 w)
{
	return vec2(z.x * w.x - z.y * w.y, z.x * w.y + z.y * w.x);
}

const float inv_root2 = 0.707106781186547524400844362104849039284835937688474036588;

// 8-roots of unity
vec2 w[8] = vec2[8]
(
	vec2(       1.0,       0.0),
	vec2( inv_root2,  inv_root),
	vec2(       0.0,       1.0),
	vec2(-inv_root2,  inv_root),
	vec2(      -1.0,       0.0),
	vec2(-inv_root2, -inv_root),
	vec2(       0.0,      -1.0),
	vec2( inv_root2, -inv_root),
)


vec2 x[8];
vec2 X[8];

// =============================  Radix 8 FFT step =============================
/* 
	X[0] = w[0] * x[0] + w[0] * x[1] + w[0] * x[2] + w[0] * x[3] + w[0] * x[4] + w[0] * x[5] + w[0] * x[6] + w[0] * x[7];
	X[1] = w[0] * x[0] + w[1] * x[1] + w[2] * x[2] + w[3] * x[3] + w[4] * x[4] + w[5] * x[5] + w[6] * x[6] + w[7] * x[7];
	X[2] = w[0] * x[0] + w[2] * x[1] + w[4] * x[2] + w[6] * x[3] + w[0] * x[4] + w[2] * x[5] + w[4] * x[6] + w[6] * x[7];
	X[3] = w[0] * x[0] + w[3] * x[1] + w[6] * x[2] + w[1] * x[3] + w[4] * x[4] + w[7] * x[5] + w[2] * x[6] + w[5] * x[7];
	X[4] = w[0] * x[0] + w[4] * x[1] + w[0] * x[2] + w[4] * x[3] + w[0] * x[4] + w[4] * x[5] + w[0] * x[6] + w[4] * x[7];
	X[5] = w[0] * x[0] + w[5] * x[1] + w[2] * x[2] + w[7] * x[3] + w[4] * x[4] + w[1] * x[5] + w[6] * x[6] + w[3] * x[7];
	X[6] = w[0] * x[0] + w[6] * x[1] + w[4] * x[2] + w[2] * x[3] + w[0] * x[4] + w[6] * x[5] + w[4] * x[6] + w[2] * x[7];
	X[7] = w[0] * x[0] + w[7] * x[1] + w[6] * x[2] + w[5] * x[3] + w[4] * x[4] + w[3] * x[5] + w[2] * x[6] + w[1] * x[7];
*/




void main()
{
	DFT8(X[0], X[1], X[2], X[3], X[4], X[5], X[6], X[7]) = 
	  = BUTTERFLY(
				DFT4(X[0], X[2], X[4], X[6]),
				DFT4(X[1], X[3], X[5], X[7])
			) 
	  = BUTTERFLY(
				BUTTERFLY(
						DFT2(X[0], X[4]),
						DFT2(X[2], X[6])
					),
				BUTTERFLY(
						DFT2(X[1], X[5])
						DFT2(X[3], X[7])
					)
				)

	X[0] = w[0] * x[0] + w[0] * x[2] + w[0] * x[4] + w[0] * x[6] +
	 	 + w[0] * x[1] + w[0] * x[3] + w[0] * x[5] + w[0] * x[7];

	X[1] = w[0] * x[0] + w[2] * x[2] + w[4] * x[4] + w[6] * x[6] +
		 + w[1] * (x[1] + w[2] * x[3] + w[4] * x[5] + w[6] * x[7]);

	X[2] = w[0] * x[0] + w[4] * x[2] + w[0] * x[4] + w[4] * x[6] +
		 + w[2] * x[1] + w[6] * x[3] + w[2] * x[5] + w[6] * x[7]

	X[3] = w[0] * x[0] + w[6] * x[2] + w[4] * x[4] + w[2] * x[6] +
		 + w[3] * x[1] + w[1] * x[3] + w[7] * x[5] + w[5] * x[7];

	X[4] = w[0] * x[0] + w[0] * x[2] + w[0] * x[4] + w[0] * x[6] + 
		 + w[4] * x[1] + w[4] * x[3] + w[4] * x[5] + w[4] * x[7];


	X[5] = w[0] * x[0] + w[2] * x[2] + w[4] * x[4] + w[6] * x[6] +
		 + w[5] * x[1] + w[7] * x[3] + w[1] * x[5] + w[3] * x[7];

	X[6] = w[0] * x[0] + w[4] * x[2] + w[0] * x[4] + w[4] * x[6] +
		 + w[6] * x[1] + w[2] * x[3] + w[6] * x[5] + w[2] * x[7];

	X[7] = w[0] * x[0] + w[7] * x[1] + w[6] * x[2] + w[5] * x[3] + w[4] * x[4] + w[3] * x[5] + w[2] * x[6] + w[1] * x[7];
	X[7] = w[0] * x[0] + w[7] * x[1] + w[6] * x[2] + w[5] * x[3] + w[4] * x[4] + w[3] * x[5] + w[2] * x[6] + w[1] * x[7];
}