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
#include "dxt.hpp"
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
    
	// ==================================================================================================================================================================================================================
	// Loading from png image file
	// ==================================================================================================================================================================================================================

    GLuint png(const char* file_name)
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
	    };

	    // row_pointers is for pointing to image_data for reading the png with libpng
        png_byte ** row_pointers = (png_byte **)malloc(height * sizeof(png_byte *));
        if (!row_pointers)
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	        free(image_data);
            fclose(fp);
    		debug_msg("Error: could not allocate memory for PNG row pointers\n");
            return 0;
		};
    
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

	GLuint dds(const char * file_name)
	{

		//===============================================================================================================================================================================================================
		// make sure file exists
		//===============================================================================================================================================================================================================
    	debug_msg("Loading DDS texture : %s", file_name);

    	FILE * file = fopen(file_name, "rb");
    	if (!file)
    	{
    		debug_msg("%s could not be opened.", file_name);
    		return 0;
    	};

		//===============================================================================================================================================================================================================
		// try to get the header first
		//===============================================================================================================================================================================================================
		dds_header header;		

		if ((fread(&header, sizeof(dds_header), 1, file) < sizeof(dds_header)) || (header.dwMagic != four_cc('D','D','S',' ')))
		{
    		debug_msg("%s is not a valid DDS format.", file_name);
    		return 0;

		};

		//===============================================================================================================================================================================================================
		// if everything went fine, read the whole file
		//===============================================================================================================================================================================================================
    	unsigned int bufsize = (header.dwMipMapCount > 1) ? header.dwPitchOrLinearSize * 2 : header.dwPitchOrLinearSize;;
        unsigned char * buffer = (unsigned char*) malloc (bufsize * sizeof(unsigned char)); 
    	fread(buffer, 1, bufsize, file); 
    	fclose(file);																											

        unsigned int components, format, block_size;

    	if (four_cc('D','X','T','1') == header.sPixelFormat.dwFourCC)
		{
			components = 3;
			block_size = 8;
    		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; 
		}
		else
		{
			components = 4;
			block_size = 16;
	    	if (four_cc('D','X','T','3') == header.sPixelFormat.dwFourCC) format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; 
    		else if (four_cc('D','X','T','5') == header.sPixelFormat.dwFourCC) format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; 
    		else 
			{
    			debug_msg("File %s format is invalid.", file_name);
				free(buffer); 
    			return 0; 
	    	};
		};			
			
		//===============================================================================================================================================================================================================
		// finally, create the texture
		//===============================================================================================================================================================================================================

    	GLuint texture_id;																									
		glGenTextures(1, &texture_id);
    	
    	glBindTexture(GL_TEXTURE_2D, texture_id);																			
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);	
		
    	unsigned int offset = 0;
    
		unsigned int width = header.dwWidth;    	
		unsigned int height = header.dwHeight;    	
    	for (unsigned int level = 0; (level < header.dwMipMapCount) && (width || height); ++level) 									// load the mipmaps
    	{ 
    		unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * block_size; 
    		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset); 
    		offset += size; 
			if (width > 1) width >>= 1;
			if (height > 1) height >>= 1;
    	}; 
    
		free(buffer); 

    	debug_msg("Texture %s successfully loaded. Width = %u. Height = %u. Mipmap levels = %u.", file_name, header.dwWidth, header.dwHeight, header.dwMipMapCount);

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
    
    
    
	
    
    
    
	
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	
	
    
    
    
    
	
    
    
    
	
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	
	
    
    
    
    
	
    
    
    
	
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	
    
	
	
    
    
    
    
	
    
    
    
	
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	
    
	
    
    
    
    
	
    
    