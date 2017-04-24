#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include <GL/glew.h> 														                                                // OpenGL extensions
#include <GLFW/glfw3.h>														                                                // windows and event management library
#include <glm/glm.hpp>

#include "log.hpp"
#include "util.hpp"

namespace util
{

	char* read_file(const char* filename)
	{
        FILE* input = fopen(filename, "rb");
        if (!input) 
        {
            debug_msg("File %s failed to open.", filename);
            return 0;
        };
        if (fseek(input, 0, SEEK_END) == -1) 
        {
            debug_msg("End of the file %s not found.", filename);
            return 0;
        };
        long int size = ftell(input);
        if (size == -1) 
        {
            debug_msg("File %s is empty.", filename);
            return 0;
        };
        
        if (fseek(input, 0, SEEK_SET) == -1) 
        {
            debug_msg("File %s reading error.", filename);
            return 0;
        };
  	  	
        char * content = (char*) malloc ((size_t) size + 1);
        if (!content) return 0;
        fread(content, 1, (size_t)size, input);
        if (ferror(input)) 
        {
            debug_msg("File %s reading error.", filename);
            free(content);
            return 0;
        };
        fclose(input);
        content[size] = '\0';
        return content;
	};



	// ==================================================================================================================================================================================================================
	// log opengl information for the current implementation
	// ==================================================================================================================================================================================================================

	bool loadOBJ(const char * file_name, GLuint &vbo, GLuint &ebo, size_t &nElems)
	{
		std::ifstream file(file_name);
	    
		if (!file.is_open())
		    debug_msg("Failed to find obj file : %s", file_name);
		
		std::vector<glm::vec3> tmpPos, tmpNorm;																					//Temporary storage for the data we read in
		std::vector<glm::vec2> tmpUv;
		
		std::map<std::string, GLushort> vertexIndices;																			// A map to associate a unique vertex with its index
		
		std::vector<glm::vec3> vertexData;																						// The final ordered packed vertices and indices
		std::vector<GLushort> indices;
	    
		std::string line;
	    
		while (std::getline(file, line))
		{
			if (line.empty()) continue;
			//Parse vertex info: positions, uv coords and normals
	    
			if (line.at(0) == 'v')
			{
				//positions
				if (line.at(1) == ' ')
				{
					glm::vec3 vec3d;
					sscanf(line.c_str(), "%*s %f %f %f", &vec3d.x, &vec3d.y, &vec3d.z);
					tmpPos.push_back(vec3d);
				}
				else if (line.at(1) == 't')
				{
					glm::vec2 vec2d;
					sscanf(line.c_str(), "%*s %f %f", &vec2d.x, &vec2d.y);
					tmpUv.push_back(vec2d);
				}
				else if (line.at(1) == 'n'){
					glm::vec3 vec3d;
					sscanf(line.c_str(), "%*s %f %f %f", &vec3d.x, &vec3d.y, &vec3d.z);
					tmpNorm.push_back(vec3d);
				};
			}		
			else if (line.at(0) == 'f')									//Parse faces
			{
				std::string face[3];
	    
	    
				size_t prev = line.find(" ", 0);
				size_t next = prev;
				for (int i = 0; i < 3; ++i)
				{
					next = line.find(" ", prev + 1);
					face[i] = line.substr(prev + 1, next - prev - 1);
					prev = next;
				};
	    
	    
	    
	    
				for (int i = 0; i < 3; ++i)
				{
					std::map<std::string, GLushort>::iterator fnd = vertexIndices.find(face[i]);
					//If we find the vertex already in the list re-use the index
					//If not we create a new vertex and index
					if (fnd != vertexIndices.end())
					{
						indices.push_back(fnd->second);
					}
					else 
					{
						unsigned int position_index, texcoord_index, normal_index;
						sscanf(face[i].c_str(), "%u/%u/%u", &position_index, &texcoord_index, &normal_index);
	    
						//Pack the position, normal and uv into the vertex data, note that obj data is
						//1-indexed so we subtract 1
						vertexData.push_back(tmpPos[position_index - 1]);
						vertexData.push_back(tmpNorm[normal_index - 1]);
						vertexData.push_back(glm::vec3(tmpUv[texcoord_index - 1], 0));
						//Store the new index, also subract 1 b/c size 1 => idx 0 and divide by 3 b/c there are 3 components per vertex
						indices.push_back((vertexData.size() - 1) / 3);
						vertexIndices[face[i]] = indices.back();
					};
				};
			};
		};
	    
		nElems = indices.size();
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(glm::vec3), &vertexData[0], GL_STATIC_DRAW);
	    
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), &indices[0], GL_STATIC_DRAW);
	    
		return true;
	};

	struct gl_integral_info
	{
		GLenum name;
		const char * enum_name;
		const char * description;
	};


	static gl_integral_info gl_state_integral_values[] =
	{   

		//===============================================================================================================================================================================================================
		// buffer binding state
		//===============================================================================================================================================================================================================
		
		{GL_ARRAY_BUFFER_BINDING, "GL_ARRAY_BUFFER_BINDING",
					"The name of the buffer object currently bound to the target GL_ARRAY_BUFFER."},
		{GL_COPY_READ_BUFFER_BINDING, "GL_COPY_READ_BUFFER_BINDING",
					"The buffer that is currently bound to the copy read bind point."},
		{GL_COPY_WRITE_BUFFER_BINDING, "GL_COPY_WRITE_BUFFER_BINDING",
					"The buffer that is currently bound to the copy write bind point."},
		{GL_ARRAY_BUFFER_BINDING, "GL_ARRAY_BUFFER_BINDING",
					"The name of the buffer object currently bound to the target GL_ARRAY_BUFFER."},
		{GL_DRAW_INDIRECT_BUFFER_BINDING, "GL_DRAW_INDIRECT_BUFFER_BINDING",
					"The name of the buffer object currently bound to the target GL_DRAW_INDIRECT_BUFFER."},
		{GL_ELEMENT_ARRAY_BUFFER_BINDING, "GL_ELEMENT_ARRAY_BUFFER_BINDING",
					"The name of the buffer object currently bound to the target GL_ELEMENT_ARRAY_BUFFER."},
		{GL_QUERY_BUFFER_BINDING, "GL_QUERY_BUFFER_BINDING",
					"The buffer that is currently bound to the query bind point."},
		{GL_TEXTURE_BUFFER_BINDING, "GL_TEXTURE_BUFFER_BINDING",
					"The buffer that is currently bound to the generic texture bind point, or 0 for none."},
		{GL_VERTEX_ARRAY_BINDING, "GL_VERTEX_ARRAY_BINDING", 
					"The name of the vertex array object currently bound to the context."},
			
		//===============================================================================================================================================================================================================
		// limits for the current implementation
		//===============================================================================================================================================================================================================
		
		{GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, "GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS",
					"The maximum number of shader storage buffer binding points on the context."},
		{GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, "GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS",
					"The maximum number of components to write to a single buffer in interleaved mode."},
		{GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS",
					"The maximum number of separate attributes or outputs that can be captured in transform feedback."},
		{GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS",
					"The maximum number of buffer objects to write with transform feedback."},
//		{GL_TRANSFORM_FEEDBACK_PAUSED, "GL_TRANSFORM_FEEDBACK_PAUSED",
//					"Whether transform feedback is paused on this object."},
//		{GL_TRANSFORM_FEEDBACK_ACTIVE, "GL_TRANSFORM_FEEDBACK_ACTIVE ",
//					"Whether transform feedback is active on this object."},

		//===============================================================================================================================================================================================================
		// framebuffer characteristics
		//===============================================================================================================================================================================================================
		{GL_DEPTH_FUNC, "GL_DEPTH_FUNC",
					"The depth comparison function."},
		{GL_DEPTH_TEST, "GL_DEPTH_TEST",
					"Whether depth testing of fragments is enabled."},
		{GL_DEPTH_WRITEMASK, "GL_DEPTH_WRITEMASK", 
					"Indicates whether the depth buffer is enabled for writing."},
		{GL_DOUBLEBUFFER, "GL_DOUBLEBUFFER",
					"Whether double buffering is supported."},
		{GL_DRAW_BUFFER, "GL_DRAW_BUFFER",
					"Which buffers are being drawn to. This is selected from the currently bound GL_DRAW_FRAMEBUFFER. The initial value is GL_BACK if there are back buffers, otherwise it is GL_FRONT."},
		{GL_DRAW_FRAMEBUFFER_BINDING, "GL_DRAW_FRAMEBUFFER_BINDING",
					"The framebuffer object currently bound to the GL_DRAW_FRAMEBUFFER target. If the default framebuffer is bound, this value will be zero."},
		{GL_MAX_COLOR_ATTACHMENTS, "GL_MAX_COLOR_ATTACHMENTS",
					"Maximum number of framebuffer attachment points for color buffers."},
		{GL_MAX_COLOR_TEXTURE_SAMPLES, "GL_MAX_COLOR_TEXTURE_SAMPLES",
					"The maximum number of samples for all color formats in a multisample texture."},
		{GL_MAX_DEPTH_TEXTURE_SAMPLES, "GL_MAX_DEPTH_TEXTURE_SAMPLES",
					"The maximum number of samples in a multisample depth or depth-stencil texture."},
		{GL_MAX_DRAW_BUFFERS , "GL_MAX_DRAW_BUFFERS ",
					"The maximum number of simultaneous outputs that may be written in a fragment shader."},
		{GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, "GL_MAX_DUAL_SOURCE_DRAW_BUFFERS",
					"The maximum number of active draw buffers when using dual-source blending."},
		{GL_MAX_FRAMEBUFFER_HEIGHT, "GL_MAX_FRAMEBUFFER_HEIGHT",
					"The maximum height for a framebuffer that has no attachments."},
		{GL_MAX_FRAMEBUFFER_LAYERS, "GL_MAX_FRAMEBUFFER_LAYERS",
					"The maximum number of layers for a framebuffer that has no attachments."},
		{GL_MAX_FRAMEBUFFER_SAMPLES, "GL_MAX_FRAMEBUFFER_SAMPLES",
					"The maximum samples in a framebuffer that has no attachments."},
		{GL_MAX_FRAMEBUFFER_WIDTH, "GL_MAX_FRAMEBUFFER_WIDTH",
					"The maximum width for a framebuffer that has no attachments."},
		{GL_MAX_INTEGER_SAMPLES, "GL_MAX_INTEGER_SAMPLES",
					"The maximum number of samples supported in integer format multisample buffers."},
		{GL_MAX_SAMPLES, "GL_MAX_SAMPLES",
					"The maximum number of samples supported for all non-integer formats."}

	};


	void gl_info()
	{
	
		//===============================================================================================================================================================================================================
		// string openGL constants
		//===============================================================================================================================================================================================================

	    debug_msg("GL_VENDOR = %s.", glGetString(GL_VENDOR));                                       
    	debug_msg("GL_RENDERER = %s.", glGetString(GL_RENDERER));                                   
	    debug_msg("GL_VERSION = %s.", glGetString(GL_VERSION));                                     
    	debug_msg("GL_SHADING_LANGUAGE_VERSION = %s.", glGetString(GL_SHADING_LANGUAGE_VERSION));   
	    debug_msg("GL_EXTENSIONS = %s.", glGetString(GL_EXTENSIONS));                               
		
		for (unsigned int i = 0; i < sizeof(gl_state_integral_values) / sizeof(gl_integral_info); ++i)
		{
			GLint value;
			glGetIntegerv(gl_state_integral_values[i].name, &value);
			debug_msg("%s = %d.\n\tDescription : %s", gl_state_integral_values[i].enum_name, value, gl_state_integral_values[i].description);
		};



	};

	// ==================================================================================================================================================================================================================
	// log opengl information for the current implementation
	// ==================================================================================================================================================================================================================

	void APIENTRY glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, const GLvoid *userParam)
	{
		static const char * UNDEFINED = "undefined";
		const char * SEVERITY;
		const char * SOURCE;
		const char * TYPE;
	    
		switch (source)
		{
			case GL_DEBUG_SOURCE_API:             SOURCE = "API"; break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   SOURCE = "Window system"; break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER: SOURCE = "Shader compiler"; break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:     SOURCE = "Third party"; break;
			case GL_DEBUG_SOURCE_APPLICATION:     SOURCE = "Application"; break;
		  default:
		    SOURCE = UNDEFINED;	  	
		};
	    
		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:               TYPE = "Error"; break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:	TYPE = "Deprecated behavior"; break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  TYPE = "Undefined behavior";	break;
			case GL_DEBUG_TYPE_PORTABILITY:         TYPE = "Portability"; break;
			case GL_DEBUG_TYPE_PERFORMANCE:         TYPE = "Performance"; break;
		  default:
		    TYPE = UNDEFINED;	  	
		};
	    
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:   SEVERITY = "High"; break;
			case GL_DEBUG_SEVERITY_MEDIUM: SEVERITY = "Medium"; break;
			case GL_DEBUG_SEVERITY_LOW:    SEVERITY = "Low"; break;
		  default:
		    SEVERITY = UNDEFINED;	  	
		};
	    
		debug_msg("OpenGL debug message : source = %s, type = %s, id = %u, severity = %s.\n\t\t msg : %s\n\t\ttime = %f.", SOURCE, TYPE, id, SEVERITY, msg, glfwGetTime());
	};

}; // namespace util

