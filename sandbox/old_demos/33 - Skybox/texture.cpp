//=======================================================================================================================================================================================================================
//  Texture loading functions implementation
//=======================================================================================================================================================================================================================

#include <cstdlib>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <png.h>
#include <jpeglib.h>    
#include <jerror.h>

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture.hpp"
#include "log.hpp"

namespace texture
{

	// ==================================================================================================================================================================================================================
	// Random texture generation
	// ==================================================================================================================================================================================================================

	GLuint random_rgb(GLuint size)
	{
		GLuint texture_id;
    	glGenTextures(1, &texture_id);    
    	glBindTexture(GL_TEXTURE_2D, texture_id);

        unsigned int data_size = size * size * sizeof(glm::vec3);
        glm::vec3 * random_data = (glm::vec3 *) malloc(data_size);

        for (unsigned int i = 0 ; i < size * size ; i++) 
			random_data[i] = glm::vec3(glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f));
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, glm::value_ptr(random_data[0]));
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);        
		glGenerateMipmap(GL_TEXTURE_2D);

        free(random_data);
        
		return texture_id;
	};
    
	GLuint random_rgba(GLuint size)
	{
		GLuint texture_id;
    	glGenTextures(1, &texture_id);    
    	glBindTexture(GL_TEXTURE_2D, texture_id);

        unsigned int data_size = size * size * sizeof(glm::vec4);
        glm::vec4 * random_data = (glm::vec4 *) malloc(data_size);

        for (unsigned int i = 0 ; i < size * size ; i++)
			random_data[i] = glm::vec4(glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f), glm::linearRand (0.0f, 1.0f));
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(random_data[0]));
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);        
		glGenerateMipmap(GL_TEXTURE_2D);

        free(random_data);
        
		return texture_id;
	};
    
	GLuint random_normal(GLuint size)
	{
		GLuint texture_id;
    	glGenTextures(1, &texture_id);    
    	glBindTexture(GL_TEXTURE_2D, texture_id);

		glm::vec3 z = glm::vec3(0.5f, 0.5f, 1.0f);
		
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, glm::value_ptr(z));
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);        
		glGenerateMipmap(GL_TEXTURE_2D);
		return texture_id;	



	};



	//===================================================================================================================================================================================================================
	// creates GL_TEXTURE_CUBE_MAP from six png files
	//===================================================================================================================================================================================================================

    GLuint cubemap_png(const char* file_names[6])
    {
		debug_msg("Loading Cubemap PNG texture.");

		static const char * face_name[] = 
		{
			"GL_TEXTURE_CUBE_MAP_POSITIVE_X",
			"GL_TEXTURE_CUBE_MAP_NEGATIVE_X",
			"GL_TEXTURE_CUBE_MAP_POSITIVE_Y",
			"GL_TEXTURE_CUBE_MAP_NEGATIVE_Y",
			"GL_TEXTURE_CUBE_MAP_POSITIVE_Z",
			"GL_TEXTURE_CUBE_MAP_NEGATIVE_Z"
		};

        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        int bit_depth, color_type;																							
        png_uint_32 width, height;
        png_byte * image_data;
        png_byte ** row_pointers;
        int rowbytes;
        GLint format;


		for (int i = 0; i < 6; ++i)
		{

		    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		    png_infop info_ptr = png_create_info_struct(png_ptr);
        	if (setjmp(png_jmpbuf(png_ptr))) 
	    	{
		        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        	    debug_msg("Error from libpng.");
            	return 0;
		    };
			// ==========================================================================================================================================================================================================
			// open the file and read its 8-byte header to make sure the format is png
			// ==========================================================================================================================================================================================================
			const char * file_name = file_names[i];
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
			debug_msg("Loading %s face from %s", face_name[i], file_name);

            png_byte header[8];
	        FILE * file = fopen(file_name, "rb");
            if ((0 == file) || (8 != fread(header, 1, 8, file)) || png_sig_cmp(header, 0, 8))
            { 
                fclose(file);
            	debug_msg("Failed to open %s file or invalid png format.", file_name);
            	return 0;
            };
	        
			// ==============================================================================================================================================================================================================
			// set up reading struct, image info struct and error callback point
			// ==============================================================================================================================================================================================================
            png_init_io(png_ptr, file);																							
            png_set_sig_bytes(png_ptr, 8);																						
            png_read_info(png_ptr, info_ptr);																					
	        
			// ==============================================================================================================================================================================================================
			// calculate the space needed and allocate it, row size in bytes should be aligned to the next 4-byte boundary as required (by default) by glTexImage2d 
			// ==============================================================================================================================================================================================================
            if (0 == i)
            {
            	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);					            	
            	if ((bit_depth != 8) || (width != height) || ((PNG_COLOR_TYPE_RGB != color_type) && (PNG_COLOR_TYPE_RGB_ALPHA != color_type)))
	            {
    	            png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        	        fclose(file);
			        glDeleteTextures(1, &texture_id);
    				debug_msg("Invalid bit depth (%d) or color type (%d) or image is not rectangular (%d x %d).", bit_depth, color_type, width, height);
					return 0;
	            };
		        png_read_update_info(png_ptr, info_ptr);																			

                format = (PNG_COLOR_TYPE_RGB == color_type) ? GL_RGB : GL_RGBA;
            	rowbytes = (png_get_rowbytes(png_ptr, info_ptr) + 3) & 0xFFFFFFFC;												
            	image_data = (png_byte *) malloc(rowbytes * height * sizeof(png_byte));						
	            row_pointers = (png_byte **) malloc(height * sizeof(png_byte *));										
            
                if ((0 == image_data) || (0 == row_pointers))
                {
	                free(image_data);
	                free(row_pointers);
                    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                    fclose(file);
			        glDeleteTextures(1, &texture_id);
                    debug_msg("Error: could not allocate memory for png image data or row pointers\n");
    				return 0;
	            };
            	for (unsigned int i = 0; i < height; i++) row_pointers[height - 1 - i] = image_data + i * rowbytes;					
	        }
	        else
	        {
		        int bit_depth0, color_type0;																							
        		png_uint_32 width0, height0;
            	png_get_IHDR(png_ptr, info_ptr, &width0, &height0, &bit_depth0, &color_type0, 0, 0, 0);
		        png_read_update_info(png_ptr, info_ptr);																			

                if ((bit_depth0 != bit_depth) || (color_type0 != color_type) || (width0 != width) || (height0 != height))
                {
	                free(image_data);
	                free(row_pointers);
                    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
                    fclose(file);
			        glDeleteTextures(1, &texture_id);
                    debug_msg("Image bit depth, color type, width or height mismatch for images %s and %s", file_names[i], file_names[0]);
    				return 0;
	            };
	        };

			// ==============================================================================================================================================================================================================
			// read the image data and fill the corresponding cubemap face
			// ==============================================================================================================================================================================================================
            png_read_image(png_ptr, row_pointers);																				
        	glTexImage2D(face, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
	    	fclose(file);

	        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		};

        free(image_data);
        free(row_pointers);

		debug_msg("Cubemap texture with id = %d successfully created.", texture_id);

        return texture_id;
	};

	//===================================================================================================================================================================================================================
	// creates GL_TEXTURE_2D from a png file
	//===================================================================================================================================================================================================================

    GLuint png(const char* file_name)
    {
		debug_msg("Loading PNG texture : %s", file_name);

		// ==============================================================================================================================================================================================================
		// open the file and read its 8-byte header to make sure the format is png
		// ==============================================================================================================================================================================================================
        png_byte header[8];
	    FILE * file = fopen(file_name, "rb");
        if ((0 == file) || (8 != fread(header, 1, 8, file)) || png_sig_cmp(header, 0, 8))
        { 
            fclose(file);
        	debug_msg("Failed to open %s or invalid png format.", file_name);
        	return 0;
        };
	
		// ==============================================================================================================================================================================================================
		// set up reading struct, image info struct and error callback point
		// ==============================================================================================================================================================================================================
	    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
    	png_infop info_ptr = png_create_info_struct(png_ptr);
        if (setjmp(png_jmpbuf(png_ptr))) 
	    {
		    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
            debug_msg("Error from libpng.");
        	return 0;
		};

        png_init_io(png_ptr, file);																							
        png_set_sig_bytes(png_ptr, 8);																						
        png_read_info(png_ptr, info_ptr);																					
        int bit_depth, color_type;																							
        png_uint_32 width, height;
        png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);					            	
    
        if ((bit_depth != 8) || ((PNG_COLOR_TYPE_RGB != color_type) && (PNG_COLOR_TYPE_RGB_ALPHA != color_type)))
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, 0);
            fclose(file);
    		debug_msg("Invalid bit depth (%d) or color type (%d).", bit_depth, color_type);
			return 0;
        };
	    png_read_update_info(png_ptr, info_ptr);																			

		// ==============================================================================================================================================================================================================
		// calculate the space needed and allocate it, row size in bytes should be aligned to the next 4-byte boundary as required (by default) by glTexImage2d 
		// ==============================================================================================================================================================================================================
        int rowbytes = (png_get_rowbytes(png_ptr, info_ptr) + 3) & 0xFFFFFFFC;												
        png_byte * image_data = (png_byte *) malloc(rowbytes * height * sizeof(png_byte));						
        png_byte ** row_pointers = (png_byte **) malloc(height * sizeof(png_byte *));										

        if ((0 == image_data) || (0 == row_pointers))
        {
	        free(image_data);
	        free(row_pointers);
            png_destroy_read_struct(&png_ptr, &info_ptr, 0);
            fclose(file);
            debug_msg("Error: could not allocate memory for PNG image data or row pointers\n");
    		return 0;
	    };
        
        for (unsigned int i = 0; i < height; i++) row_pointers[height - 1 - i] = image_data + i * rowbytes;					

		// ==============================================================================================================================================================================================================
		// finally, read the image data, create a texture and supply it with the data
		// ==============================================================================================================================================================================================================
        png_read_image(png_ptr, row_pointers);																				

        GLint format = (PNG_COLOR_TYPE_RGB == color_type) ? GL_RGB : GL_RGBA;
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glGenerateMipmap(GL_TEXTURE_2D);
	
		// ==============================================================================================================================================================================================================
		// clean up
		// ==============================================================================================================================================================================================================
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        free(image_data);
        free(row_pointers);
	    fclose(file);
        return texture_id;
    };
    
	























	// ==================================================================================================================================================================================================================
	// Loading from bmp image file
	// ==================================================================================================================================================================================================================

    GLuint bmp(const char * file_name)
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
    	};
    
		// Read the header, i.e. the 54 first bytes
    
    	// If less than 54 bytes are read, problem
    	if ((fread(header, 1, 54, file) != 54) || (header[0] != 'B') || (header[1] != 'M') || (*(int*) &(header[0x1E]) != 0) || (*(int*) &(header[0x1C]) != 24))
		{ 
    		debug_msg("Not a correct BMP file");
    		return 0;
    	};
    
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
    };
    
	// ==================================================================================================================================================================================================================
	// Loading from dds (direct draw surface) image file
	// ==================================================================================================================================================================================================================

    #define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
	#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
    #define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII
        
	GLuint dds(const char * file_name)
    {
    
    	debug_msg("Loading DDS texture : %s", file_name);
    
    	FILE *fp; 
    	fp = fopen(file_name, "rb"); 																						// try to open the file
    	if (!fp)
    	{
    		debug_msg("%s could not be opened.", file_name);
    		return 0;
    	};
    	
    	char filecode[4]; 																									// verify the type of file
    	fread(filecode, 1, 4, fp); 
    	if (strncmp(filecode, "DDS ", 4) != 0) 
		{
    		debug_msg("%s is not a valid DDS file.", file_name);
			fclose(fp); 
			return 0; 
		};
    	
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
    	};
    	
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
    	}; 
    
		free(buffer); 
		return texture_id;
    };
    
	// ==================================================================================================================================================================================================================
	// Loading from jpg
	// ==================================================================================================================================================================================================================

    GLuint jpg(const char * file_name)
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
		};
    
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
    	};
    
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
    	};
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
    };

};	// namespace texture
    
    
    
	
    
    
    
	
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	
	
    
    
    
    
	
    
    
    
	
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	
	
    
    
    
    
	
    
    
    
	
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	
    
	
	
    
    
    
    
	
    
    
    
	
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	
    
	
    
    
    
    
	
    
    