#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <png.h>

#include <jpeglib.h>    
#include <jerror.h>

#include "log.hpp"


GLuint png_texture_load(const char* file_name)
{
	debug_msg("Loading PNG texture : %s", file_name);

    png_byte header[8];

    FILE * fp = fopen(file_name, "rb");
    if (!fp)
    {
        debug_msg("Failed to open %s.", file_name);
        return 0;
    };

    // read the header
    fread(header, 1, 8, fp);

    if (png_sig_cmp(header, 0, 8))
    {
        fclose(fp);
        debug_msg("Error: %s is not a PNG.\n", file_name);
		return 0;
    };

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fclose(fp);
        debug_msg("Error: png_create_read_struct returned 0.\n");
		return 0;
    };

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        debug_msg("error: png_create_info_struct returned 0.\n");
		return 0;
    };

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        debug_msg("Error: png_create_info_struct returned 0.\n");
		return 0;
    };

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) 
	{
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        debug_msg("Error from libpng\n");
		return 0;
    };

    
    png_init_io(png_ptr, fp);																						// init png reading
    png_set_sig_bytes(png_ptr, 8);																					// let libpng know you already read the first 8 bytes
    png_read_info(png_ptr, info_ptr);																				// read all the info up to the image data
    
    int bit_depth, color_type;																						// variables to pass to get info
    png_uint_32 width, height;

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);					            // get info about png

    if (bit_depth != 8) debug_msg("%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);

    GLint format;
	if (PNG_COLOR_TYPE_RGB == color_type)
		format = GL_RGB;
	else if (PNG_COLOR_TYPE_RGB_ALPHA == color_type)
		format = GL_RGBA;
	else
	{
		debug_msg("%s: Unknown libpng color type %d.\n", file_name, color_type);
		return 0;
	};
		  		
    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data = (png_byte *)malloc(rowbytes * height * sizeof(png_byte)+15);
    if (!image_data)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        debug_msg("Error: could not allocate memory for PNG image data\n");
		return 0;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_byte ** row_pointers = (png_byte **)malloc(height * sizeof(png_byte *));
    if (!row_pointers)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
		debug_msg("Error: could not allocate memory for PNG row pointers\n");
        return 0;
	}

    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < height; i++) row_pointers[height - 1 - i] = image_data + i * rowbytes;
    
    png_read_image(png_ptr, row_pointers);																				// read the png into image_data through row_pointers

    // Generate the OpenGL texture object
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
    return texture;
}


GLuint bmp_texture_load(const char * file_name)
{

	debug_msg("Loading BMP texture : %s", file_name);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(file_name,"rb");
	if (!file)
	{
		debug_msg("%s could not be opened.", file_name); 
		return 0;
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ((fread(header, 1, 54, file) != 54) || (header[0] != 'B') || (header[1] != 'M') || (*(int*) &(header[0x1E]) != 0) || (*(int*) &(header[0x1C]) != 24))
	{ 
		debug_msg("Not a correct BMP file");
		return 0;
	}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (!imageSize) imageSize = width * height * 3;			// 3 : one byte for each Red, Green and Blue component
	if (!dataPos) dataPos = 54;								// The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	
	fread(data, 1, imageSize, file);						// Read the actual data from the file into the buffer
	fclose (file);											// Everything is in memory now, the file wan be closed

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete [] data;

	// Poor filtering, or ...
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII


GLuint dds_texture_load(const char * file_name)
{

	debug_msg("Loading DDS texture : %s", file_name);

	FILE *fp; 
	fp = fopen(file_name, "rb"); 																						// try to open the file
	if (!fp)
	{
		debug_msg("%s could not be opened.", file_name);
		return 0;
	}
	
	char filecode[4]; 																									// verify the type of file
	fread(filecode, 1, 4, fp); 
	if (strncmp(filecode, "DDS ", 4) != 0) 
	{
		debug_msg("%s is not a valid DDS file.", file_name);
		fclose(fp); 
		return 0; 
	}
	
	unsigned char header[124];
	fread(&header, 124, 1, fp); 																						// get the surface description

	unsigned int height      = *(unsigned int*)&(header[8 ]);
	unsigned int width	     = *(unsigned int*)&(header[12]);
	unsigned int linearSize	 = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int fourCC      = *(unsigned int*)&(header[80]);
 
	unsigned char * buffer;
	unsigned int bufsize;
	
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize; 															// calculate the size with all including the mipmaps
	buffer = (unsigned char*) malloc (bufsize * sizeof(unsigned char)); 
	fread(buffer, 1, bufsize, fp); 
	fclose(fp);																											// close the file pointer

	unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4; 
	unsigned int format;
	if (FOURCC_DXT1 == fourCC)
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; 
	else if (FOURCC_DXT3 == fourCC)
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; 
	else if (FOURCC_DXT5 == fourCC)
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; 
	else
	{
		debug_msg("Unsupported DDS format.");
		free(buffer); 
		return 0; 
	}
	
	GLuint texture_id;																									// create a new OpenGL texture identificator
	glGenTextures(1, &texture_id);
	
	glBindTexture(GL_TEXTURE_2D, texture_id);																			// bind the newly created texture -- all future texture functions will modify this texture
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);	
	
	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16; 
	unsigned int offset = 0;

	
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level) 									// load the mipmaps
	{ 
		unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize; 
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset); 
		offset += size; 
		width  /= 2; 
		height /= 2; 
		if(width < 1) width = 1;																						// deal with Non-Power-Of-Two textures
		if(height < 1) height = 1;
	} 

	free(buffer); 
	return texture_id;
}

GLuint jpg_texture_load (const char * file_name)
{
	debug_msg("Loading JPG texture : %s", file_name);

	unsigned long data_size;																			// length of the file
	int channels;               																		// 3 => RGB, 4 => RGBA 
	unsigned int type;  
	char * rowptr;    // pointer to an array
	char * jdata;        // data for the image

	struct jpeg_decompress_struct info;																	// for our jpeg info
	struct jpeg_error_mgr err;          																// the error handler

	FILE* file = fopen(file_name, "rb");  																// open the file

	info.err = jpeg_std_error(& err);     
	jpeg_create_decompress(& info);																		// fills info structure


	if(!file) 																							//if the jpeg file doesn't load
	{
   		debug_msg("Error reading JPEG file %s!", file_name);
		return 0;
	}

	jpeg_stdio_src(&info, file);    
	jpeg_read_header(&info, 1);																			// read jpeg file header
	jpeg_start_decompress(&info);																		// decompress the file

	
//	x = info.output_width;																				//set width and height
//	y = info.output_height;
//	channels = info.num_components;
	GLenum format; 
	if (info.num_components == 3)
		format = GL_RGB;
	else if (info.num_components == 4)
		format = GL_RGBA;
	else
	{
   		debug_msg("JPEG file is neither RGB, nor RGBA %s!", file_name);
		return 0;
	}

	data_size = info.output_width * info.output_height * info.num_components;

	//--------------------------------------------
	// read scanlines one at a time & put bytes 
	//    in jdata[] array. Assumes an RGB image
	//--------------------------------------------
	jdata = (char *) malloc(data_size);
	while (info.output_scanline < info.output_height) // loop
	{
  		// Enable jpeg_read_scanlines() to fill our jdata array
		rowptr = (char *) jdata + info.num_components * info.output_width * info.output_scanline;  // secret to method
		jpeg_read_scanlines(&info, &rowptr, 1);
	}
	//---------------------------------------------------

	jpeg_finish_decompress(&info);   //finish decompressing

	//----- create OpenGL tex map  --------
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, format, info.output_width, info.output_height, 0, format, GL_UNSIGNED_BYTE, jdata);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	fclose(file);																						// close the file
	free(jdata);

	return texture_id;																					// for OpenGL tex maps
}