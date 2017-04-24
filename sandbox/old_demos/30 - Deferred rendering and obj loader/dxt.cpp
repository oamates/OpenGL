#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dxt.hpp"

//=======================================================================================================================================================================================================================
// set USE_COVARIANCE_MATRIX = 1 to use the covarince matrix method...
//=======================================================================================================================================================================================================================
#define USE_COVARIANCE_MATRIX 1

//=======================================================================================================================================================================================================================
// helper functions prototypes
//=======================================================================================================================================================================================================================

//=======================================================================================================================================================================================================================
// Takes a 4x4 block of pixels and compresses it into 8 bytes in DXT1 format (color only, no alpha).
//=======================================================================================================================================================================================================================
void compress_dds_color_block(int channels, const unsigned char *const uncompressed, unsigned char compressed[8]);

//=======================================================================================================================================================================================================================
// Takes a 4x4 block of pixels and compresses the alpha component it into 8 bytes for use in DXT5 dds files. Speed is valued over prettyness, at least for now.
//=======================================================================================================================================================================================================================
void compress_dds_alpha_block(const unsigned char * const uncompressed, unsigned char compressed[8]);

//=======================================================================================================================================================================================================================
// Makes an int out of four unsigned bytes
//=======================================================================================================================================================================================================================
unsigned int four_cc(unsigned char p, unsigned char q, unsigned char r, unsigned char s);




//=======================================================================================================================================================================================================================
// stores image into a dds file
//=======================================================================================================================================================================================================================
bool dxt_compress_save(const char* file_name, int width, int height, int channels, const unsigned char * const data)
{
	// ==================================================================================================================================================================================================================
	// validate arguments
	// ==================================================================================================================================================================================================================
	if((0 == filename) || (width < 1) || (height < 1) || (channels < 1) || (channels > 4) || (0 == data)) return false;

	// ==================================================================================================================================================================================================================
	// convert data
	// ==================================================================================================================================================================================================================
	int size;
	bool alpha = channels & 1;
	unsigned char* data = alpha ? dxt1_compress(data, width, height, channels, &size): 
								  dxt5_compress(data, width, height, channels, &size);

	// ==================================================================================================================================================================================================================
	// create header
	// ==================================================================================================================================================================================================================
	dds_header header;
	memset(&header, 0, sizeof(DDS_header));																							
	header.dwMagic = four_cc('D','D','S',' ');
	header.dwSize = 124;
	header.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE;
	header.dwWidth = width;
	header.dwHeight = height;
	header.dwPitchOrLinearSize = size;
	header.sPixelFormat.dwSize = 32;
	header.sPixelFormat.dwFlags = DDPF_FOURCC;
	header.sPixelFormat.dwFourCC = alpha ? four_cc('D','X','T','1') : four_cc('D','X','T','5');
	header.sCaps.dwCaps1 = DDSCAPS_TEXTURE;
	
	// ==================================================================================================================================================================================================================
	// write header + data
	// ==================================================================================================================================================================================================================
	FILE* f = fopen(file_name, "wb");																									
	fwrite(&header, sizeof(dds_header), 1, f);
	fwrite(data, 1, size, f);
	fclose(f);
	
	// ==================================================================================================================================================================================================================
	// clean up and exit
	// ==================================================================================================================================================================================================================
	free(data);
	return true;
};


//=======================================================================================================================================================================================================================
// DXT1 compression function
//=======================================================================================================================================================================================================================
unsigned char* dxt1_compress(const unsigned char * const uncompressed, int width, int height, int channels, int& size)
{
	// ==================================================================================================================================================================================================================
	// validate arguments
	// ==================================================================================================================================================================================================================
	if((width < 1) || (height < 1) || (0 == uncompressed) || (channels < 1) || (channels > 4)) return 0;

	// ==================================================================================================================================================================================================================
	// calculate the size required and allocate memory (8 bytes per 4x4 pixel block)
	// ==================================================================================================================================================================================================================
	size = ((width + 3) >> 2) * ((height + 3) >> 2) * 8;																		
	unsigned char* compressed = (unsigned char*) malloc(*out_size);
	unsigned char ublock[48];
	unsigned char cblock[8];
	
	// ==================================================================================================================================================================================================================
	// main loop running through each block
	// ==================================================================================================================================================================================================================
	int index = 0;
	int block_count = 0;
	int step = (channels < 3) ? 0 : 1; 																								

	for(int j = 0; j < height; j += 4)																							
	{
		for(int i = 0; i < width; i += 4)
		{
			
			int idx = 0;																											// copy this block into a new one
			int my = (j + 4 >= height) ? (height - j) : 4;
			int mx = (i + 4 >= width) ? (width - i) : 4;

			for(int y = 0; y < my; ++y)
			{
				for(int x = 0; x < mx; ++x)
				{
					ublock[idx++] = uncompressed[(j + y) * width * channels + (i + x) * channels];
					ublock[idx++] = uncompressed[(j + y) * width * channels + (i + x) * channels + step];
					ublock[idx++] = uncompressed[(j + y) * width * channels + (i + x) * channels + step + step];
				};
				for(x = mx; x < 4; ++x)
				{
					ublock[idx++] = ublock[0];
					ublock[idx++] = ublock[1];
					ublock[idx++] = ublock[2];
				};
			};
			for(int y = my; y < 4; ++y)
			{
				for(int x = 0; x < 4; ++x)
				{
					ublock[idx++] = ublock[0];
					ublock[idx++] = ublock[1];
					ublock[idx++] = ublock[2];
				};
			};

			++block_count;																											// compress the block
			compress_dds_color_block(3, ublock, cblock);
			
			for(int x = 0; x < 8; ++x) compressed[index++] = cblock[x];																// copy the data from the block into the main block
		};
	};

	return compressed;
};

//=======================================================================================================================================================================================================================
// DXT5 compression function
//=======================================================================================================================================================================================================================
unsigned char* dxt5_compress(const unsigned char *const uncompressed, int width, int height, int channels, int* out_size)
{
	// ==================================================================================================================================================================================================================
	// validate arguments
	// ==================================================================================================================================================================================================================
	if((width < 1) || (height < 1) || (0 == uncompressed) || (channels < 1) || ( channels > 4)) return 0;

	unsigned char ublock[64];
	unsigned char cblock[8];
	
	*out_size = ((width + 3) >> 2) * ((height + 3) >> 2) * 16;																		// get the RAM for the compressed image (16 bytes per 4x4 pixel block)
	unsigned char* compressed = (unsigned char*) malloc(*out_size);
	
	// ==================================================================================================================================================================================================================
	// main loop running through each block
	// ==================================================================================================================================================================================================================	
	int index = 0;
	int block_count = 0;
	int step = (channels < 3) ? 0 : 1;																								// for channels == 1 or 2, I do not step forward for R,G,B values

	for(int j = 0; j < height; j += 4)																								// run through each block
	{
		for(int i = 0; i < width; i += 4)
		{
			/*	local variables, and my block counter	*/
			int idx = 0;
			int my = (j + 4 >= height) ? (height - j) : 4;
			int mx = (i + 4 >= width) ? (width - i) : 4;

			for(int y = 0; y < my; ++y )
			{
				for(int x = 0; x < mx; ++x )
				{
					ublock[idx++] = uncompressed[(j + y) * width * channels + (i + x) * channels];
					ublock[idx++] = uncompressed[(j + y) * width * channels + (i + x) * channels + chan_step];
					ublock[idx++] = uncompressed[(j + y) * width * channels + (i + x) * channels + chan_step + chan_step];			// channels = 1 or 3 have no alpha, 2 or 4 do have alpha
					ublock[idx++] = (channels & 1) ? 255 : (uncompressed[(j + y) * width * channels + (i + x) * channels + channels - 1]);
				}
				for(int x = mx; x < 4; ++x )
				{
					ublock[idx++] = ublock[0];
					ublock[idx++] = ublock[1];
					ublock[idx++] = ublock[2];
					ublock[idx++] = ublock[3];
				};
			};
			for(int y = my; y < 4; ++y )
			{
				for(int x = 0; x < 4; ++x )
				{
					ublock[idx++] = ublock[0];
					ublock[idx++] = ublock[1];
					ublock[idx++] = ublock[2];
					ublock[idx++] = ublock[3];
				};
			};
			
			compress_alpha_block(ublock, cblock);																                // now compress the alpha block
			for( x = 0; x < 8; ++x ) compressed[index++] = cblock[x];																//	copy the data from the compressed alpha block into the main buffer
			
			
			++block_count;																											// then compress the color block
			compress_color_block(4, ublock, cblock);																			// copy the data from the compressed color block into the main buffer
			
			for( x = 0; x < 8; ++x ) compressed[index++] = cblock[x];
		};
	};
	return compressed;
};


void compute_color_line_STDEV(const unsigned char *const uncompressed, int channels, float point[3], float direction[3])
{

	// ==================================================================================================================================================================================================================
	// calculate mean and second moments needed for the covariance matrix
	// ==================================================================================================================================================================================================================

	int isum_r = 0, isum_g = 0, isum_b = 0;
	int isum_rr = 0, isum_gg = 0, isum_bb = 0,
	    isum_rg = 0, isum_rb = 0, isum_gb = 0;
	
	for(int i = 0; i < 16 * channels; i += channels)
	{
		isum_r  += uncompressed[i + 0];
		isum_g  += uncompressed[i + 1];
		isum_b  += uncompressed[i + 2];

		isum_rr += uncompressed[i + 0] * uncompressed[i + 0];
		isum_gg += uncompressed[i + 1] * uncompressed[i + 1];
		isum_bb += uncompressed[i + 2] * uncompressed[i + 2];
		isum_rg += uncompressed[i + 0] * uncompressed[i + 1];
		isum_rb += uncompressed[i + 0] * uncompressed[i + 2];
		isum_gb += uncompressed[i + 1] * uncompressed[i + 2];
	};

	const float inv_16 = 1.0f / 16.0f;

	float sum_r = isum_r * inv_16;																									// convert the sums to averages
	float sum_g = isum_g * inv_16;
	float sum_b = isum_b * inv_16;
	
	float sum_rr -= (isum_r * isum_r) * inv_16;																						// and convert the squares to the squares of the value - avg_value
	float sum_gg -= (isum_g * isum_g) * inv_16;
	float sum_bb -= (isum_b * isum_b) * inv_16;
	float sum_rg -= (isum_r * isum_g) * inv_16;
	float sum_rb -= (isum_r * isum_b) * inv_16;
	float sum_gb -= (isum_g * isum_b) * inv_16;
	
	point[0] = sum_r;																												// the point on the color line is the average
	point[1] = sum_g;
	point[2] = sum_b;

  #if USE_COVARIANCE_MATRIX
	sum_r = 1.0f;																													// use the covariance matrix directly, 1st iteration
	sum_g = 2.718281828f;
	sum_b = 3.141592654f;

	direction[0] = sum_r * sum_rr + sum_g * sum_rg + sum_b * sum_rb;
	direction[1] = sum_r * sum_rg + sum_g * sum_gg + sum_b * sum_gb;
	direction[2] = sum_r * sum_rb + sum_g * sum_gb + sum_b * sum_bb;
	
	sum_r = direction[0];																											// 2nd iteration, use results from the 1st guy
	sum_g = direction[1];
	sum_b = direction[2];
	direction[0] = sum_r * sum_rr + sum_g * sum_rg + sum_b * sum_rb;
	direction[1] = sum_r * sum_rg + sum_g * sum_gg + sum_b * sum_gb;
	direction[2] = sum_r * sum_rb + sum_g * sum_gb + sum_b * sum_bb;
	
	sum_r = direction[0];																											// 3rd iteration, use results from the 2nd guy
	sum_g = direction[1];
	sum_b = direction[2];
	direction[0] = sum_r * sum_rr + sum_g * sum_rg + sum_b * sum_rb;
	direction[1] = sum_r * sum_rg + sum_g * sum_gg + sum_b * sum_gb;
	direction[2] = sum_r * sum_rb + sum_g * sum_gb + sum_b * sum_bb;
  #else
	
	direction[0] = sqrt(sum_rr);																									// use standard deviation method (very robust, a tiny bit slower and less accurate)
	direction[1] = sqrt(sum_gg);                                                                                                    
	direction[2] = sqrt(sum_bb);                                                                                                    
	                                                                                                                                
	if( sum_gg > sum_rr )																											// which has a greater component
	{                                                                                                                               
		if( sum_rg < 0.0f ) direction[0] = -direction[0];																			// green has greater component, so base the other signs off of green
		if( sum_gb < 0.0f ) direction[2] = -direction[2];                                                                           
	}                                                                                                                               
	else                                                                                                                            
	{                                                                                                                               
		if( sum_rg < 0.0f ) direction[1] = -direction[1];																			// red has a greater component	
		if( sum_rb < 0.0f ) direction[2] = -direction[2];
	};
  #endif
};

void LSE_master_colors_max_min(int *cmax, int *cmin, int channels, const unsigned char *const uncompressed)
{

	if((channels < 3) || (channels > 4)) return;																					// error check

	int c0[3], c1[3];																												// the master colors
	
	float sum_x[] = {0.0f, 0.0f, 0.0f};																								// used for fitting the line
	float sum_x2[] = {0.0f, 0.0f, 0.0f};
	float dot;

	compute_color_line_STDEV( uncompressed, channels, sum_x, sum_x2 );
	float vec_len2 = 1.0f / (0.00001f + sum_x2[0] * sum_x2[0] + sum_x2[1] * sum_x2[1] + sum_x2[2] * sum_x2[2]);
	
	float dot_max = sum_x2[0] * uncompressed[0] + sum_x2[1] * uncompressed[1] + sum_x2[2] * uncompressed[2];						// finding the max and min vector values
	float dot_min = dot_max;

	for(int i = 1; i < 16; ++i )
	{
		dot = sum_x2[0] * uncompressed[i * channels + 0] + sum_x2[1] * uncompressed[i * channels + 1] + sum_x2[2] * uncompressed[i * channels + 2];
		if(dot < dot_min) dot_min = dot;
		if(dot > dot_max ) dot_max = dot;
	};
	
	dot = sum_x2[0] * sum_x[0] + sum_x2[1] * sum_x[1] + sum_x2[2] * sum_x[2];														// and the offset (from the average location)
	dot_min -= dot;
	dot_max -= dot;
	
	dot_min *= vec_len2;																											// post multiply by the scaling factor
	dot_max *= vec_len2;
	
	for(int i = 0; i < 3; ++i )																										// OK, build the master colors
	{
		c0[i] = (int)(0.5f + sum_x[i] + dot_max * sum_x2[i]);																		// color 0
		if(c0[i] < 0) c0[i] = 0;
		if(c0[i] > 255) c0[i] = 255;
		c1[i] = (int)(0.5f + sum_x[i] + dot_min * sum_x2[i]);																		// color 1
		if(c1[i] < 0) c1[i] = 0; 
		if(c1[i] > 255) c1[i] = 255;
	};
	
	int i = rgb_to_565(c0[0], c0[1], c0[2]);																						// down_sample (with rounding?)
	int j = rgb_to_565(c1[0], c1[1], c1[2]);

	if(i > j)
	{
		*cmax = i;
		*cmin = j;
	}
	else
	{
		*cmax = j;
		*cmin = i;
	};
};

void compress_color_block(int channels, const unsigned char *const uncompressed, unsigned char compressed[8])
{
	int enc_c0, enc_c1;
	int c0[4], c1[4];
	float color_line[] = {0.0f, 0.0f, 0.0f, 0.0f};
	
	int swizzle4[] = {0, 2, 3, 1};																									// stupid order
	
	LSE_master_colors_max_min(&enc_c0, &enc_c1, channels, uncompressed);															// get the master colors
	
	compressed[0] = (enc_c0 >> 0) & 255;																							// store the 565 color 0 and color 1
	compressed[1] = (enc_c0 >> 8) & 255;
	compressed[2] = (enc_c1 >> 0) & 255;
	compressed[3] = (enc_c1 >> 8) & 255;
	
	compressed[4] = 0;																												// zero out the compressed data
	compressed[5] = 0;
	compressed[6] = 0;
	compressed[7] = 0;
	
	rgb_888_from_565( enc_c0, &c0[0], &c0[1], &c0[2] );																				// reconstitute the master color vectors
	rgb_888_from_565( enc_c1, &c1[0], &c1[1], &c1[2] );
	
	float vec_len2 = 0.0f;																											// the new vector
	for(int i = 0; i < 3; ++i)
	{
		color_line[i] = (float)(c1[i] - c0[i]);
		vec_len2 += color_line[i] * color_line[i];
	};

	if(vec_len2 > 0.0f) vec_len2 = 1.0f / vec_len2;
	
	color_line[0] *= vec_len2;																										// pre-proform the scaling
	color_line[1] *= vec_len2;
	color_line[2] *= vec_len2;
	
	float dot_offset = color_line[0]*c0[0] + color_line[1]*c0[1] + color_line[2]*c0[2];												// compute the offset (constant) portion of the dot product
	
	int next_bit = 32;																												// store the rest of the bits
	for(int i = 0; i < 16; ++i)
	{
		int next_value = 0;																											// find the dot product of this color, to place it on the line (should be [-1,1])
		float dot_product = color_line[0] * uncompressed[i * channels + 0] +
			                color_line[1] * uncompressed[i * channels + 1] +
			                color_line[2] * uncompressed[i * channels + 2] - dot_offset;											// map to [0,3]

		next_value = (int)(dot_product * 3.0f + 0.5f);
		
		if(next_value > 3) next_value = 3;
		if(next_value < 0) next_value = 0;																							// OK, store this value
		
		compressed[next_bit >> 3] |= swizzle4[ next_value ] << (next_bit & 7);
		next_bit += 2;
	};																																// done compressing to DXT1
};

void compress_dds_alpha_block(const unsigned char * const uncompressed, unsigned char compressed[8])
{
	int swizzle8[] = {1, 7, 6, 5, 4, 3, 2, 0};																						// stupid order
	
	int a0 = uncompressed[3];																										// get the alpha limits (a0 > a1)
	int a1 = a0;

	for(int i = 7; i < 64; i += 4)
	{
		if(uncompressed[i] > a0)
			a0 = uncompressed[i];
		else 
			if(uncompressed[i] < a1) a1 = uncompressed[i];
	};
	
	compressed[0] = a0;																												// store those limits, and zero the rest of the compressed dataset
	compressed[1] = a1;
	compressed[2] = 0;																												// zero out the compressed data
	compressed[3] = 0;
	compressed[4] = 0;
	compressed[5] = 0;
	compressed[6] = 0;
	compressed[7] = 0;

	int next_bit = 16;																												// store all the alpha values
	float scale_me = 7.9999f / (a0 - a1);
	for(int i = 3; i < 64; i += 4)
	{
		int svalue;																													// convert this alpha value to a 3 bit number
		int value = (int)((uncompressed[i] - a1) * scale_me);
		svalue = swizzle8[value & 7];
		compressed[next_bit >> 3] |= svalue << (next_bit & 7);																		// OK, store this value, start with the 1st byte
		if((next_bit & 7) > 5) compressed[1 + (next_bit >> 3)] |= svalue >> (8 - (next_bit & 7) );									// spans 2 bytes, fill in the start of the 2nd byte
		next_bit += 3;
	};																																// done compressing to DXT1
};

//=======================================================================================================================================================================================================================
// helper functions implementation
//=======================================================================================================================================================================================================================

int convert_bit_range(int c, int from_bits, int to_bits)
{
	int b = (1 << (from_bits - 1)) + c * ((1 << to_bits) - 1);
	return (b + (b >> from_bits)) >> from_bits;
};

int rgb_to_565( int r, int g, int b )
{
	return (convert_bit_range(r, 8, 5) << 11) | (convert_bit_range(g, 8, 6 ) << 05) | (convert_bit_range(b, 8, 5) << 00);
};

void rgb_888_from_565(unsigned int c, int *r, int *g, int *b)
{
	*r = convert_bit_range((c >> 11) & 31, 5, 8);
	*g = convert_bit_range((c >> 05) & 63, 6, 8);
	*b = convert_bit_range((c >> 00) & 31, 5, 8);
};

unsigned int four_cc(unsigned char p, unsigned char q, unsigned char r, unsigned char s)
{
	return (p << 0) | (q << 8) | (r << 16) | (s << 24);	
};



//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================
//=======================================================================================================================================================================================================================







//=======================================================================================================================================================================================================================
// decode a DXT color block
//=======================================================================================================================================================================================================================
void decode_dxt_color_block(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, const FORMAT format, unsigned char *src)
{
	unsigned char colors[4][3];
	unsigned short c0 = *(unsigned short *) src;
	unsigned short c1 = *(unsigned short *) (src + 2);

	// ==================================================================================================================================================================================================================
	// extract the two stored colors
	// ==================================================================================================================================================================================================================
	colors[0][0] = ((c0 >> 11) & 0x1F) << 3;
	colors[0][1] = ((c0 >>  5) & 0x3F) << 2;
	colors[0][2] =  (c0        & 0x1F) << 3;

	colors[1][0] = ((c1 >> 11) & 0x1F) << 3;
	colors[1][1] = ((c1 >>  5) & 0x3F) << 2;
	colors[1][2] =  (c1        & 0x1F) << 3;

	// ==================================================================================================================================================================================================================
	// compute the other two colors
	// ==================================================================================================================================================================================================================
	if ((c0 > c1) || (format == FORMAT_DXT5))
	{
		for (int i = 0; i < 3; i++)
		{
			colors[2][i] = (2 * colors[0][i] +     colors[1][i] + 1) / 3;
			colors[3][i] = (    colors[0][i] + 2 * colors[1][i] + 1) / 3;
		};
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
			colors[3][i] = 0;
		};
	};

	// ==================================================================================================================================================================================================================
	// compute the other two colors
	// ==================================================================================================================================================================================================================
	src += 4;
	for (int y = 0; y < h; y++)
	{
		unsigned char* dst = dest + yOff * y;
		unsigned int indexes = src[y];
		for (int x = 0; x < w; x++)
		{
			unsigned int index = indexes & 0x3;
			dst[0] = colors[index][0];
			dst[1] = colors[index][1];
			dst[2] = colors[index][2];
			indexes >>= 2;
			dst += xOff;
		};
	};
};

//=======================================================================================================================================================================================================================
// decode a DXT3 alpha block
//=======================================================================================================================================================================================================================
void decode_dxt3_alpha_block(unsigned char* dest, const int w, const int h, const int xOff, const int yOff, unsigned char* src)
{
	for (int y = 0; y < h; y++)
	{
		unsigned char* dst = dest + yOff * y;
		unsigned int alpha = ((ushort *) src)[y];
		for (int x = 0; x < w; x++)
		{
			*dst = (alpha & 0xF) * 17;
			alpha >>= 4;
			dst += xOff;
		};
	};
};

//=======================================================================================================================================================================================================================
// decode a DXT5 alpha block
//=======================================================================================================================================================================================================================
void decode_dxt5_alpha_block(unsigned char* dest, const int w, const int h, const int xOff, const int yOff, unsigned char* src)
{
	unsigned char a0 = src[0];
	unsigned char a1 = src[1];
	unsigned long long alpha = (*(unsigned long long *) src) >> 16;

	for (int y = 0; y < h; y++)
	{
		unsigned char *dst = dest + yOff * y;
		for (int x = 0; x < w; x++)
		{
			int k = ((unsigned int) alpha) & 0x07;
			*dst = (k == 0) ? a0 :
			       (k == 1) ? a1 :
                   (a0 > a1) ? (ubyte)(((8 - k) * a0 + (k - 1) * a1) / 7) : 
				   (k == 6)	? 0 :
				   (k == 7) ? 255 : (unsigned char) (((6 - k) * a0 + (k - 1) * a1) / 5);
			alpha >>= 3;
			dst += xOff;
		};
		if (w < 4) alpha >>= (3 * (4 - w));
	};
};

// Decodes DXT and 3Dc formats
void decodeCompressedImage(unsigned char* dest, unsigned char* src, const int width, const int height, const FORMAT format)
{
	int sx = (width  < 4) ? width  : 4;
	int sy = (height < 4) ? height : 4;
	int nChannels = getChannelCount(format);
	for (int y = 0; y < height; y += 4)
	{
		for (int x = 0; x < width; x += 4)
		{
			unsigned char *dst = dest + (y * width + x) * nChannels;
			if (format == FORMAT_DXT3)
			{
				decodeDXT3AlphaBlock(dst + 3, sx, sy, 4, width * 4, src);
				src += 8;
			}
			else if (format == FORMAT_DXT5)
			{
				decodeDXT5AlphaBlock(dst + 3, sx, sy, 4, width * 4, src);
				src += 8;
			}
			if (format <= FORMAT_DXT5)
			{
				decodeColorBlock(dst, sx, sy, nChannels, width * nChannels, format, src);
				src += 8;
			}
			else
			{
				if (format == FORMAT_ATI1N)
				{
					decodeDXT5AlphaBlock(dst, sx, sy, 1, width, src);
					src += 8;
				}
				else
				{
					decodeDXT5AlphaBlock(dst,     sx, sy, 2, width * 2, src + 8);
					decodeDXT5AlphaBlock(dst + 1, sx, sy, 2, width * 2, src);
					src += 16;
				};
			};
		};
	};
};

bool HE_Image::decompress()
{
	if (!isCompressedFormat(format))
	{
		printf("");
		return false;
	};

	FORMAT destFormat = (format == FORMAT_ATI1N) ? FORMAT_I8 : 
						(format == FORMAT_ATI2N) ? FORMAT_IA8 :
						(format == FORMAT_DXT1)  ? FORMAT_RGB8 : FORMAT_RGBA8;

	unsigned char* newPixels = new ubyte[getMipMappedSize(0, nMipMaps, destFormat)];

	int level = 0;
	unsigned char *src, *dst = newPixels;
	while ((src = getPixels(level)) != 0)
	{
		int w = getWidth(level);
		int h = getHeight(level);
		int d = (depth == 0)? 6 : getDepth(level);

		int dstSliceSize = getImageSize(destFormat, w, h, 1, 1);
		int srcSliceSize = getImageSize(format,     w, h, 1, 1);

		for (int slice = 0; slice < d; slice++)
		{
			decodeCompressedImage(dst, src, w, h, format);
			dst += dstSliceSize;
			src += srcSliceSize;
		};
		level++;
	};
  
	format = destFormat;
	delete pixels;
	pixels = newPixels;
	return true;
}